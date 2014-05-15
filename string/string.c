#include "string_buffer.h" /* TODO move StringBuffer* to separate compilation unit */

/* @return the string or NULL when memory allocation fails or string length was exceeded */
String*
String_init(const char* source, size_t nchars)
{
	if (nchars > STRING_MAX_LENGTH)
	{
		XFERR("Maximum string length (%llu) exceeded: %zu", STRING_MAX_LENGTH, nchars);
		return NULL;
	}

	String* s;

	if (source)
	{
		s = S_alloc(nchars);
		if (s == NULL)
			return NULL;   /* memory allocation failure */
		memcpy(s->value, source, nchars);
		s->length = nchars;
	}
	else
	{
		s = S_alloc(nchars);
		if (s == NULL)
			return NULL;            /* memory allocation failure */
		s->length = 0;
	}

	XFDBG("Created string of length %zu (used %zu)", nchars, s->length);
	S_nullterm(s);
	return s;
}

BufferStatus
String_shift(String* s, size_t count)
{
	XFDBG("Shifting %zu tokens of string with length:%zu", count, s->length);

	if (count > s->length)
	{
		XFERR("Shift count %zu exceed string length %zu", count, s->length);
		return CX_ERR;
	}
	else
	{
		size_t nremaining = s->length - count;
		if (nremaining > 0)
			memcpy(s->value, s->value + count, nremaining);
		/* zero unused data */
		size_t nunused = s->length - nremaining;
		memset(s->value + nremaining, nunused, 0);
		s->length = nremaining;
	}

	S_nullterm(s);
	return CX_OK;
}

inline void
StringBuffer_init(StringBuffer* buffer, size_t length)
{
	buffer->length = length;
	buffer->string = String_init(NULL, length);
	if (buffer->string == NULL)
		StringBuffer_set_error(buffer, STRING_ERROR_ERRNO);
}

StringBuffer*
StringBuffer_new(size_t length)
{
	StringBuffer* buf = cx_alloc(sizeof(StringBuffer));

	StringBuffer_init(buf, length);
	return buf;
}

StringBuffer*
StringBuffer_from_string(String* string)
{
	StringBuffer* buf = cx_alloc(sizeof(StringBuffer));

	buf->string = string;
	return buf;
}

inline void
StringBuffer_free_members(StringBuffer* buffer)
{
	S_free(buffer->string);
}

void
StringBuffer_free(StringBuffer* buffer)
{
	if (buffer)
	{
		StringBuffer_free_members(buffer);
		cx_free(buffer);
	}
}

void
StringBuffer_set_error(StringBuffer* buffer, StringBufferError error)
{
	if (buffer->error == STRING_ERROR_ERRNO)
		XFDBG("Error in string bufffer %s", strerror(errno));
	else
		XFDBG("Error %d in string buffer", buffer->error);

	buffer->error = error;
}

/*
 * @return CX_OK on succes, CX_ERR on failure
 */
BufferStatus
StringBuffer_make_room(StringBuffer* buffer, size_t offset, size_t nlength_requested)
{
	String* old_string = buffer->string;
	size_t old_length = buffer->length;

	/* offset index must be within range */
	if (offset > old_length)
	{
		StringBuffer_set_error(buffer, STRING_ERROR_INVALID_OFFSET);
		return CX_ERR;
	}

	size_t new_length = offset + nlength_requested;

	/* check if we have to grow */
	if (new_length > old_length)
	{
		String* new_string = String_init(NULL, new_length);

		/* memory allocation failure */
		if (new_string == NULL)
			return CX_ERR;

		buffer->string = new_string;
		buffer->length = new_length;
		StringBuffer_ncat(buffer, old_string->value, old_string->length);
		S_free(old_string);

		XFDBG("Resized buffer from %zu -> %zu", old_length, new_length);
	}

	return CX_OK;
}

BufferStatus
StringBuffer_append_number(StringBuffer* buffer, size_t offset, uint64_t num, size_t nbytes)
{
	return StringBuffer_append(buffer, offset, (char*)&num, nbytes);
}

BufferStatus
StringBuffer_append(StringBuffer* buffer, size_t offset, const char* source, size_t nchars)
{
	XFDBG("\n	buffer[length:%zu, used:%zu, unused:%zu] source[nchars:%zu]",
	      StringBuffer_length(buffer), StringBuffer_used(buffer), StringBuffer_unused(buffer), nchars);

	XFDBG("append nchars:%zu at offset:%zu", nchars, offset);

	if (StringBuffer_make_room(buffer, offset, nchars) < 0)
		return CX_ERR;

	memcpy(StringBuffer_value(buffer) + offset, source, nchars);
	buffer->string->length = offset + nchars;
	S_nullterm(buffer->string);

	return CX_OK;
}

/* @return the number of bytes read or CX_ERR on error */
ssize_t
StringBuffer_read(StringBuffer* buffer, size_t offset, int fd, ssize_t nchars)
{
	assert(nchars > 0); /* application bug */

	if (StringBuffer_make_room(buffer, offset, (size_t)nchars) < 0)
		return CX_ERR;

	ssize_t nread = read(fd, S_term(buffer->string), (size_t)nchars);

	XFDBG("Read %zd (read size %zu) chars into buffer", nread, nchars);

	if (nread < 0)
	{
		StringBuffer_set_error(buffer, STRING_ERROR_ERRNO);
		return CX_ERR;
	}
	else if (nread > 0)
	{
		buffer->string->length = offset + (size_t)nread;
		S_nullterm(buffer->string);
	}

	return nread;
}

/* read until EOF or when blocking is set to 0 until EWOULDBLOCK,
 * @ return CX_ERR on error, 0 if no data was loaded, CX_OK else
 */
BufferStatus
StringBuffer_fdxload(StringBuffer* buffer, int fd, size_t chunk_size, int block)
{
	ssize_t nread;
	size_t ntotal = 0;

	/* read until EOF or until an error occurs */

	ssize_t original_chunk_size = chunk_size % READ_MAX;
	ssize_t chunk_size_used = original_chunk_size;


	while ((nread = StringBuffer_fdncat(buffer, fd, chunk_size_used)) > 0)
	{
		ntotal += (size_t)nread;

		/* If read returns at least one character, there is no way you can tell whether end-of-file was reached.
		 * But if you did reach the end, the next read will return zero.
		 *
		 * - TODO avoid unnecessary buffer growth and reduce the chunk size to the available buffer size
		 */
		if (nread > 0 && nread < chunk_size_used)
		{
			size_t unused = StringBuffer_unused(buffer);
			if ((size_t)chunk_size_used > unused)
			{
				ssize_t chunk_size_new = (ssize_t)unused;
				XFDBG("Read less bytes (%zu) than chunk size (%zu). Reduced read size to %zu",
				      nread, chunk_size_used, chunk_size_new);
				chunk_size_used = chunk_size_new;
			}
		}
		// TODO
	}

	XFDBG("Read %zu bytes", ntotal);

	/* check for EOF */
	if (nread == 0)
		return CX_NOOP;

	/* check error */
	if (nread < 0)
		if (block || errno != EWOULDBLOCK)
			return CX_ERR;

	return CX_OK;
}

/*
 * Load data until buffer is full, the input blocks or an error occurs.
 * @return the number of unused bytes available in the buffer, or -1 on error (see errno)
 */
BufferStatus
StringBuffer_ffill(StringBuffer* buffer, int fd, int blocking)
{
	size_t nunused;
	size_t ntotal = 0;

	while ((nunused = StringBuffer_unused(buffer)) > 0)
	{
		/* multipass read */
		ssize_t nread = StringBuffer_fdncat(buffer, fd, nunused % READ_MAX);

		XFDBG("Read %zd bytes", nread);

		if (nread == 0)
			return CX_NOOP;

		if (nread < 0)
		{
			if (blocking || errno != EWOULDBLOCK)
				return CX_ERR;
			else
				return CX_OK;
		}
	}

	return CX_OK;
}

StringBuffer*
StringBuffer_from_printf(size_t length, const char* format, ...)
{
	StringBuffer* buffer = StringBuffer_new(length);
	va_list ap;

	va_start(ap, format);
	StringBuffer_vsnprintf(buffer, 0, format, ap);
	va_end(ap);
	return buffer;
}

/* @see StringBuffer_vsnprintf */
int
StringBuffer_sprintf(StringBuffer* buffer, size_t offset, const char* format, ...)
{
	va_list ap;

	va_start(ap, format);
	int chars_printed = StringBuffer_vsnprintf(buffer, offset, format, ap);
	va_end(ap);
	return chars_printed;
}

/*
 * CX_ERR if an error occured, or >= 0 for count of printed characters
 */
int
StringBuffer_vsnprintf(StringBuffer* buffer, size_t offset, const char* format, va_list args)
{
	/* check that offset is within within range */
	if (offset > buffer->length)
	{
		StringBuffer_set_error(buffer, STRING_ERROR_INVALID_OFFSET);
		return CX_ERR;
	}

	char* string_start = S_get(buffer->string, offset);
	size_t nchars_available = buffer->length - offset;
	int nchars_printed; /* number of chars printed (excluding \0) */
	va_list ap;
	va_copy(ap, args);

	nchars_printed = vsnprintf(string_start, nchars_available + 1, format, ap);
	va_end(ap);

	/*
	 * vsnprintf never returns a negative value, instead
	 * it returns the number of characters that would have been printed
	 * if buffer would have been large enough.
	 *
	 * snprintf/vsnprintf maximum output size is restricted by the integer return type
	 * but the upper limit might be smaller.
	 * TODO check what happens when the output exceeds INT_MAX
	 * http://stackoverflow.com/questions/8119914/printf-fprintf-maximum-size-according-to-c99
	 */
	assert(nchars_printed >= 0); /* c library bug */

	/* check if buffer was to small */
	if ((size_t)nchars_printed > nchars_available)
	{
		/* expand string buffer */
		if (StringBuffer_make_room(buffer, offset, (size_t)nchars_printed) != 1)
			return CX_ERR;

		va_copy(ap, args);
		string_start = S_get(buffer->string, offset);
		nchars_available = buffer->length - offset;
		nchars_printed = vsnprintf(string_start, nchars_available + 1, format, ap);
		va_end(ap);

		XFDBG("nchars printed: %d, nchars available: %zu", nchars_printed, nchars_available);
		assert((size_t)nchars_printed == nchars_available); /* application bug */
	}

	buffer->string->length = offset + (size_t)nchars_printed;

	return nchars_printed;
}

void
StringBuffer_write_bytes_into(StringBuffer* buf, const char* const format, const uint8_t* bytes, size_t nbytes)
{
	size_t i;

	for (i = 0; i < nbytes; i++)
		StringBuffer_aprintf(buf, format, *(bytes + i));
}

void
StringBuffer_print_bytes_hex(StringBuffer* in, size_t nbytes, const char* message)
{
	size_t nbytes_max = (nbytes > StringBuffer_used(in) ? StringBuffer_used(in) : nbytes);
	static const char* const format = "0x%x ";
	StringBuffer* buf = StringBuffer_new(nbytes_max * sizeof(format));

	StringBuffer_write_bytes_into(buf, format, (uint8_t*)StringBuffer_value(in), nbytes_max);
	XFDBG("%s [%zu of %zu]: %s", message, nbytes_max, StringBuffer_used(in), StringBuffer_value(buf));
	StringBuffer_free(buf);
}

/* string pointer methods */

StringPointer*
StringPointer_new(const char* data, size_t length)
{
	StringPointer* pointer = cx_alloc(sizeof(StringPointer));

	pointer->value = data;
	pointer->length = length;
	return pointer;
}

void
StringPointer_free(StringPointer* pointer)
{
	cx_free(pointer);
}
