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

void
StringBuffer_shift(StringBuffer* buffer, size_t nshift)
{
	if (nshift == 0)
	{
		STRING_BUFFER_WARN(buffer, "shift size 0");
		return;
	}

	size_t nused = StringBuffer_used(buffer);

	if (nshift > nused)
	{
		STRING_BUFFER_FERROR(buffer, STRING_BUFFER_STATUS_ERROR_INVALID_ACCESS,
				     "shift count %zu out of bounds", nshift);
		return;
	}
	else
	{
		size_t nremaining = nused - nshift;
		char* buf_start = StringBuffer_value(buffer);
		StringBuffer_used(buffer) = nremaining;

		if (nremaining > 0)
		{
			memmove(buf_start, buf_start + nshift, nremaining);
			STRING_BUFFER_FDBG(buffer, "shifted %zu byte", nshift);
		}
		else
		{
			STRING_BUFFER_DBG(buffer, "cleared");
		}

		/* zero unused data */
		size_t nunused = nused - nremaining;
		memset(buf_start + nremaining, nunused, 0);
	}

	S_nullterm(buffer->string);
}

inline void
StringBuffer_init(StringBuffer* buffer, size_t length)
{
	buffer->length = length;
	buffer->string = String_init(NULL, length);
	if (buffer->string == NULL)
	{
		STRING_BUFFER_ERRNO(buffer);
	}
}

StringBuffer*
StringBuffer_new(size_t length)
{
	StringBuffer* buf = cx_alloc(sizeof(StringBuffer));

	StringBuffer_init(buf, length);
	STRING_BUFFER_DBG(buf, "created");

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

/* @return true if the requested size is availabe, false on error */
bool
StringBuffer_make_room(StringBuffer* buffer, size_t offset, size_t nsize_required)
{
	String* old_string = buffer->string;
	size_t old_length = buffer->length;

	size_t new_length = offset + nsize_required;

	/* check if we have to grow */
	if (new_length > old_length)
	{
		/* offset index must be within range */
		if (offset > old_length)
		{
			STRING_BUFFER_FERROR(buffer, STRING_BUFFER_STATUS_ERROR_INVALID_ACCESS,
					     "offset %zu out of bounds", offset);
			return false;
		}

		String* new_string = String_init(NULL, new_length);

		/* memory allocation failure */
		if (new_string == NULL)
		{
			STRING_BUFFER_ERRNO(buffer);
			return false;
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
		STRING_BUFFER_FDBG(buffer, "resized %zu --> %zu", old_length, new_length);
	}
	return true;
}

void
StringBuffer_append_number(StringBuffer* buffer, size_t offset, uint64_t num, size_t nsize)
{
	StringBuffer_append(buffer, offset, (char*)&num, nsize);
}

void
StringBuffer_append(StringBuffer* buffer, size_t offset, const char* source, size_t nsize)
{
	if (nsize == 0)
	{
		STRING_BUFFER_WARN(buffer, "append size is 0");
		return;
	}
	else
	{
		if (StringBuffer_make_room(buffer, offset, nsize))
		{
			memcpy(StringBuffer_value(buffer) + offset, source, nsize);
			buffer->string->length = offset + nsize;
			S_nullterm(buffer->string);
			STRING_BUFFER_FDBG(buffer, "appended %zu byte", nsize);
		}
	}
}

/* @return the number of bytes read or CX_ERR (-1) on error */
ssize_t
StringBuffer_read(StringBuffer* buffer, size_t offset, int fd, ssize_t read_size)
{
	if (read_size < 0)
	{
		STRING_BUFFER_FERROR(buffer, STRING_BUFFER_STATUS_ERROR_INVALID_READ_SIZE,
				     "negative read size: %zu", read_size);
		return CX_ERR;
	}
	else if (read_size == 0)
	{
		STRING_BUFFER_WARN(buffer, "read size is 0");
		return 0;
	}
	else
	{
		if (StringBuffer_make_room(buffer, offset, (size_t)read_size))
		{
			ssize_t nread = read(fd, S_term(buffer->string), (size_t)read_size);
			if (nread == -1)
			{
//				STRING_BUFFER_ERRNO(buffer);
				buffer->status = STRING_BUFFER_STATUS_ERROR_ERRNO;
				buffer->error_errno = errno;
				return CX_ERR;
			}
			if (nread == 0)
			{
				STRING_BUFFER_DBG(buffer, "EOF");
				buffer->status = STRING_BUFFER_STATUS_EOF;
			}
			else
			{
				STRING_BUFFER_FDBG(buffer, "read %zu byte (read size %zu)", nread, read_size);
				if (nread > 0)
				{
					buffer->string->length = offset + (size_t)nread;
					S_nullterm(buffer->string);
				}
			}
			return nread;
		}
		else
			return CX_ERR;
	}
}

/*
 * read until EOF or when blocking is set to 0 until EWOULDBLOCK
 */
void
StringBuffer_fdload(StringBuffer* buffer, int fd, size_t read_size)
{
	if (read_size == 0)
	{
		STRING_BUFFER_WARN(buffer, "read size 0");
		return;
	}

	size_t nread_total = 0;
	ssize_t nread;

	ssize_t read_size_original = read_size % READ_SIZE_MAX;
	ssize_t read_size_actual = read_size_original;

	/* read until EOF or until an error occurs */
	while ((nread = StringBuffer_fdncat(buffer, fd, read_size_actual)) > 0)
	{
		nread_total += (size_t)nread;

		/* from 'man 2 read'
		 * "If read returns at least one character, there is no way you can tell whether end-of-file was reached.
		 * But if you did reach the end, the next read will return zero."
		 *
		 * Reduce read size to remaining buffer length if
		 * actual characters read < read size
		 */
		if (nread > 0 && nread < read_size_actual)
		{
			size_t nunused = StringBuffer_unused(buffer);

			if ((size_t)read_size_actual > nunused)
			{
				ssize_t chunk_size_reduced = (ssize_t)nunused;
				STRING_BUFFER_FDBG(buffer, "reduced read size (%zu --> %zu)",
						   read_size_actual, chunk_size_reduced);
				read_size_actual = chunk_size_reduced;
			}
		}
	}

	STRING_BUFFER_FDBG(buffer, "total bytes read %zu", nread_total);
}

/*
 * Load data until buffer is full, the input blocks or an error occurs.
 */
void
StringBuffer_ffill(StringBuffer* buffer, int fd)
{
	if (StringBuffer_length(buffer) == 0 || StringBuffer_unused(buffer) == 0)
	{
//		STRING_BUFFER_WARN(buffer, "buffer full");
		return;
	}
	size_t nunused;
	size_t nread_total = 0;

	/* read until buffer is full */
	while ((nunused = StringBuffer_unused(buffer)) > 0)
	{
		ssize_t nread = StringBuffer_fdncat(buffer, fd, nunused % READ_SIZE_MAX);

		if (nread > 0)
			nread_total += (size_t)nread;
		else
			break;
	}

	STRING_BUFFER_FDBG(buffer, "total bytes read %zu", nread_total);
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
 * @return The number of printed characters or CX_ERR (-1) if an error occurred.
 */
int
StringBuffer_vsnprintf(StringBuffer* buffer, size_t offset, const char* format, va_list args)
{
	/* check that offset is within within range */
	if (offset > buffer->length)
	{
		STRING_BUFFER_FERROR(buffer, STRING_BUFFER_STATUS_ERROR_INVALID_ACCESS,
				     "offset %zu out of bounds", offset);
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

	/* if buffer was to small then grow buffer and repeat the procedure */
	if ((size_t)nchars_printed > nchars_available)
	{
		/* expand string buffer */
		if (StringBuffer_make_room(buffer, offset, (size_t)nchars_printed))
		{
			va_copy(ap, args);
			string_start = S_get(buffer->string, offset);
			nchars_available = buffer->length - offset;
			nchars_printed = vsnprintf(string_start, nchars_available + 1, format, ap);
			va_end(ap);

			assert((size_t)nchars_printed == nchars_available); /* application bug */
		}
		else
			return CX_ERR;
	}

	buffer->string->length = offset + (size_t)nchars_printed;
	STRING_BUFFER_FDBG(buffer, "printed %d byte", nchars_printed);

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
	case STRING_BUFFER_STATUS_OK: return "OK";
	case STRING_BUFFER_STATUS_EOF: return "EOF";
	case STRING_BUFFER_STATUS_ERROR_ERRNO: return "ERROR_ERRNO";
	case STRING_BUFFER_STATUS_ERROR_TO_SMALL: return "ERROR_TO_SMALL";
	case STRING_BUFFER_STATUS_ERROR_INVALID_ACCESS: return "ERROR_INVALID_ACCESS";
	case STRING_BUFFER_STATUS_ERROR_INVALID_READ_SIZE: return "ERROR_INVALID_READ_SIZE";
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
