#include "string_buffer.h" /* TODO move StringBuffer* to separate compilation unit */

String*
String_init(const char* source, size_t nchars)
{
	if (nchars > STRING_MAX_LENGTH)
		return NULL;

	String* s;

	if (source)
	{
		s = S_alloc(nchars);
		memcpy(s->value, source, nchars);
		s->length = nchars;
	}
	else
	{
		s = S_alloc(nchars);
		s->length = 0;
	}

	S_nullterm(s);
	return s;
}

int
String_shift(String* s, size_t count)
{
	XFDBG("shift count:%zu, string length:%zu", count, s->length);

	if (count == 0)
		return 0;
	if (count > s->length)
		return -1;

	if (count == s->length)
		s->length = 0;
	else
	{
		size_t remaining = s->length - count;
		memcpy(s->value, s->value + count, remaining);
		s->length = remaining;
	}
	S_nullterm(s);
	return 1;
}

inline void
StringBuffer_init(StringBuffer* buffer, size_t length)
{
	buffer->length = length;
	buffer->string = String_init(NULL, length);
}

StringBuffer*
StringBuffer_new(size_t length)
{
	if (length > STRING_MAX_LENGTH)
		return NULL;

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

/*
 * @return 0 if room is available, -1 on error, or > 0 (new length) if buffer was expanded
 */
int
StringBuffer_make_room(StringBuffer* buffer, size_t offset, size_t nlength_requested)
{
	// index must be within range
	if (offset > buffer->string->length)
		return -1;

	size_t new_length = offset + nlength_requested;

	/* enough unused bytes available */
	if (new_length <= buffer->length)
		return 0;

	if (new_length > STRING_MAX_LENGTH)
		return -1;

	String* new_string = S_realloc(buffer->string, new_length);
	if (new_string == NULL)
		return -1;

#ifdef STRING_DEBUG
	XFDBG("Incremented buffer size %zu -> %zu", buffer->length, new_length);
#endif


	buffer->string = new_string;
	buffer->length = new_length;
	return (int)new_length;
}

ssize_t
StringBuffer_append_number(StringBuffer* buffer, size_t offset, uint64_t num, size_t nbytes)
{
	return StringBuffer_append(buffer, offset, (char*)&num, nbytes);
}

ssize_t
StringBuffer_append(StringBuffer* buffer, size_t offset, const char* source, size_t nchars)
{
	XFDBG("\n	buffer[length:%zu, used:%zu, unused:%zu] source[nchars:%zu]",
	      StringBuffer_length(buffer), StringBuffer_used(buffer), StringBuffer_unused(buffer), nchars);

	XFDBG("append nchars:%zu at offset:%zu", nchars, offset);

	if (StringBuffer_make_room(buffer, offset, nchars) == -1)
		return -1;

	memcpy(StringBuffer_value(buffer) + offset, source, nchars);
	buffer->string->length = offset + nchars;
	S_nullterm(buffer->string);

	return (ssize_t)nchars;
}

ssize_t
StringBuffer_read(StringBuffer* buffer, size_t offset, int fd, size_t nchars)
{
	if (StringBuffer_make_room(buffer, offset, nchars) == -1)
		return -1;

	ssize_t nread = read(fd, S_term(buffer->string), nchars);

#ifdef STRING_DEBUG
	XFDBG("Read %zd (read size %zu) chars into buffer", nread, nchars);
#endif

	if (nread > 0)
	{
		buffer->string->length = offset + (size_t)nread;
		S_nullterm(buffer->string);
	}

	return nread;
}

ssize_t
StringBuffer_fdxload(StringBuffer* buffer, int fd, size_t chunk_size, int blocking)
{
	ssize_t total_read = 0;

	while (1)
	{
		ssize_t nread = StringBuffer_fdncat(buffer, fd, chunk_size);

		if (nread == 0)
			break;
		if (nread < 0)
		{
			if (!blocking && errno == EWOULDBLOCK)
				return total_read;
			else
				return nread;
		}

		total_read += nread;
	}
	return total_read;
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

ssize_t
StringBuffer_sprintf(StringBuffer* buffer, size_t offset, const char* format, ...)
{
	va_list ap;

	va_start(ap, format);
	ssize_t chars_printed = StringBuffer_vsnprintf(buffer, offset, format, ap);
	va_end(ap);
	return chars_printed;
}

/* -1 on error, >= 0 for count of printed characters */
ssize_t
StringBuffer_vsnprintf(StringBuffer* buffer, size_t offset, const char* format, va_list args)
{
	/* check that offset is within within range */
	if (offset > buffer->length)
		return -1;

	char* string_start = S_get(buffer->string, offset);
	size_t nchars_available = (buffer->length - offset) + 1 /* \0 */;
	size_t nchars_printed; /* number of chars printed (excluding \0) */
	va_list ap;
	va_copy(ap, args);
	nchars_printed = (size_t)vsnprintf(string_start, nchars_available, format, ap);
	va_end(ap);

	/* check if buffer was large enough */
	if (nchars_printed >= nchars_available)
	{
		StringBuffer_make_room(buffer, offset, nchars_printed);
		va_copy(ap, args);
		string_start = S_get(buffer->string, offset);
		nchars_available = (buffer->length - offset) + 1 /* \0 */;
		vsnprintf(string_start, nchars_available, format, ap);
		va_end(ap);
	}

	buffer->string->length = offset + nchars_printed;

	return (ssize_t)nchars_printed;
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
