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
StringBuffer_shift(StringBuffer* buffer, size_t count)
{
	if (count == 0)
		return CX_OK;

	size_t nused = StringBuffer_used(buffer);

	if (count > nused)
	{
		STRING_BUFFER_STATUS(buffer, STRING_BUFFER_STATUS_ERROR_INVALID_ACCESS, count);
		buffer->status_data = count;
		return CX_ERR;
	}
	else
	{
		size_t nremaining = nused - count;
		char* buf_start = StringBuffer_value(buffer);

		if (nremaining > 0)
		{
			memmove(buf_start, buf_start + count, nremaining);
			STRING_BUFFER_STATUS(buffer, STRING_BUFFER_STATUS_SHIFTED, count);
		}
		else
		{
			STRING_BUFFER_STATUS(buffer, STRING_BUFFER_STATUS_CLEARED, count);
		}

		StringBuffer_used(buffer) = nremaining;

		/* zero unused data */
		size_t nunused = nused - nremaining;
		memset(buf_start + nremaining, nunused, 0);
	}

	S_nullterm(buffer->string);
	return CX_OK;
}

inline void
StringBuffer_init(StringBuffer* buffer, size_t length)
{
	buffer->length = length;
	buffer->string = String_init(NULL, length);
	if (buffer->string == NULL)
	{
		STRING_BUFFER_STATUS(buffer, STRING_BUFFER_STATUS_ERROR_ERRNO, 0UL);
	}
}

StringBuffer*
StringBuffer_new(size_t length)
{
	StringBuffer* buf = cx_alloc(sizeof(StringBuffer));

	StringBuffer_init(buf, length);
	XFDBG("Created buffer[%p l:%zu]", buf, length);

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
		STRING_BUFFER_STATUS(buffer, STRING_BUFFER_STATUS_ERROR_INVALID_ACCESS, offset);
		return CX_ERR;
	}

	size_t new_length = offset + nlength_requested;

	/* check if we have to grow */
	if (new_length > old_length)
	{
		String* new_string = String_init(NULL, new_length);

		/* memory allocation failure */
		if (new_string == NULL)
		{
			STRING_BUFFER_STATUS(buffer, STRING_BUFFER_STATUS_ERROR_ERRNO, 0UL);
			return CX_ERR;
		}

		buffer->string = new_string;
		buffer->length = new_length;

		if (old_length > 0)
		{
			memcpy(StringBuffer_value(buffer),  old_string->value, old_string->length);
			buffer->string->length = old_string->length;
			S_nullterm(buffer->string);
		}

		S_free(old_string);

		STRING_BUFFER_STATUS(buffer, STRING_BUFFER_STATUS_RESIZED, new_length - old_length);
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
	if (nchars == 0)
	{
		XFERR("Buffer[%p] - request to append 0 chars: %s", buffer, source);
		return CX_OK;
	}

	if (StringBuffer_make_room(buffer, offset, nchars) == CX_OK)
	{
		memcpy(StringBuffer_value(buffer) + offset, source, nchars);
		buffer->string->length = offset + nchars;
		S_nullterm(buffer->string);
		STRING_BUFFER_STATUS(buffer, STRING_BUFFER_STATUS_NEW_DATA, nchars);
		return CX_OK;
	}
	else
		return CX_ERR;
}

/* @return the number of bytes read or CX_ERR on error */
ssize_t
StringBuffer_read(StringBuffer* buffer, size_t offset, int fd, ssize_t nchars)
{
	if (nchars == 0)
	{
		XERR("Request to append 0 chars");
		return CX_OK;
	}

	if (StringBuffer_make_room(buffer, offset, (size_t)nchars) < 0)
		return CX_ERR;

	ssize_t nread = read(fd, S_term(buffer->string), (size_t)nchars);

	if (nread < 0)
	{
		STRING_BUFFER_STATUS(buffer, STRING_BUFFER_STATUS_ERROR_ERRNO, 0UL);
		return CX_ERR;
	}
	else if (nread > 0)
	{
		STRING_BUFFER_STATUS(buffer, STRING_BUFFER_STATUS_NEW_DATA, (size_t)nread);
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
	if (chunk_size == 0)
	{
		XERR("Request to append 0 chars");
		return CX_OK;
	}

	size_t nread_total = 0;
	ssize_t nread;

	ssize_t chunk_size_original = chunk_size % READ_MAX;
	ssize_t chunk_size_actual = chunk_size_original;

	/* read until EOF or until an error occurs */
	while ((nread = StringBuffer_fdncat(buffer, fd, chunk_size_actual)) > 0)
	{
		nread_total += (size_t)nread;

		/* from 'man 2 read'
		 * "If read returns at least one character, there is no way you can tell whether end-of-file was reached.
		 * But if you did reach the end, the next read will return zero."
		 *
		 * Reduce read size to remaining buffer length if
		 * actual characters read < read size
		 */
		if (nread > 0 && nread < chunk_size_actual)
		{
			size_t nunused = StringBuffer_unused(buffer);

			if ((size_t)chunk_size_actual > nunused)
			{
				ssize_t chunk_size_reduced = (ssize_t)nunused;
				XFDBG("Buffer[%p] - reduced read size to %zu", buffer, chunk_size_reduced);
				chunk_size_actual = chunk_size_reduced;
			}
		}
	}

	STRING_BUFFER_STATUS(buffer, STRING_BUFFER_STATUS_NEW_DATA, (size_t)nread_total);

	/* check error */
	if (nread < 0)
	{
		if (block || errno != EWOULDBLOCK)
		{
			STRING_BUFFER_STATUS(buffer, STRING_BUFFER_STATUS_ERROR_ERRNO, 0UL);
			return CX_ERR;
		}
	}

	return CX_OK;
}

/*
 * Load data until buffer is full, the input blocks or an error occurs.
 * @return the number of unused bytes available in the buffer, or -1 on error (see errno)
 */
BufferStatus
StringBuffer_ffill(StringBuffer* buffer, int fd, int blocking)
{
	if (StringBuffer_length(buffer) == 0)
	{
		XERR("Request fill buffer of size 0");
		return CX_OK;
	}

	size_t nunused;
	size_t ntotal = 0;

	/* read until buffer is full */
	while ((nunused = StringBuffer_unused(buffer)) > 0)
	{
		ssize_t nread = StringBuffer_fdncat(buffer, fd, nunused % READ_MAX);

		if (nread < 0)
		{
			if (blocking || errno != EWOULDBLOCK)
			{
				STRING_BUFFER_STATUS(buffer, STRING_BUFFER_STATUS_ERROR_ERRNO, 0UL);
				return CX_ERR;
			}
			else
			{
				break;
			}
		}
		else if (nread == 0)
		{
			break;
		}
		else if (nread > 0)
		{
			ntotal += (size_t)nread;
		}
	}

	STRING_BUFFER_STATUS(buffer, STRING_BUFFER_STATUS_NEW_DATA, ntotal);
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
		STRING_BUFFER_STATUS(buffer, STRING_BUFFER_STATUS_ERROR_INVALID_ACCESS, offset);
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
		if (StringBuffer_make_room(buffer, offset, (size_t)nchars_printed) != CX_OK)
			return CX_ERR;

		va_copy(ap, args);
		string_start = S_get(buffer->string, offset);
		nchars_available = buffer->length - offset;
		nchars_printed = vsnprintf(string_start, nchars_available + 1, format, ap);
		va_end(ap);

		assert((size_t)nchars_printed == nchars_available); /* application bug */
	}

	buffer->string->length = offset + (size_t)nchars_printed;
	STRING_BUFFER_STATUS(buffer, STRING_BUFFER_STATUS_NEW_DATA, (size_t)nchars_printed);

	return nchars_printed;
}

void
StringBuffer_write_bytes_into(StringBuffer* buf, const char* const format, const uint8_t* bytes, size_t nbytes)
{
	size_t i;

	for (i = 0; i < nbytes; i++)
	{
		StringBuffer_aprintf(buf, format, *(bytes + i));
	}
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

const char*
cx_strstatus(StringBufferStatus status)
{
	switch (status)
	{
	case STRING_BUFFER_STATUS_ERROR_ERRNO: return "ERROR_ERRNO";
	case STRING_BUFFER_STATUS_ERROR_TO_SMALL: return "ERROR_TO_SMALL";
	case STRING_BUFFER_STATUS_ERROR_INVALID_ACCESS: return "ERROR_INVALID_ACCESS";
	case STRING_BUFFER_STATUS_CLEARED: return "CLEARED";
	case STRING_BUFFER_STATUS_SHIFTED: return "SHIFTED";
	case STRING_BUFFER_STATUS_RESIZED: return "RESIZED";
	case STRING_BUFFER_STATUS_NEW_DATA: return "NEW_DATA";
	}
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
