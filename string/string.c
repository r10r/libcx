#include "string_buffer.h" /* TODO move StringBuffer* to separate compilation unit */

String*
String_init(const char* source, size_t nchars)
{
	if (nchars > STRING_MAX_LENGTH)
		return NULL;

	String* s;
	size_t nalloc_chars = nchars;

	if (source)
	{
		/* check if source is '\0\ terminated */
		if (!is_nullterm(source, nchars))
			nalloc_chars += 1;

		s = S_alloc(nalloc_chars);
		memcpy(s->value, source, nchars);
		s->length = nalloc_chars;
	}
	else
	{
		/* create an empty '\0' terminated string */
		// FIXME check why post increment is not working with unsigned values
		if (nchars == 0)
			nalloc_chars += 1;

		s = S_alloc(nalloc_chars);
		s->length = 0;
	}
	S_nullterm(s);
	return s;
}

int
String_shift(String* s, size_t count)
{
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

	StringBuffer* buf = malloc(sizeof(StringBuffer));
	StringBuffer_init(buf, length);
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
		free(buffer);
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

	XFLOG("Incremented buffer size %zu -> %zu", buffer->length, new_length);

	buffer->string = new_string;
	buffer->length = new_length;
	return (int)new_length;
}

ssize_t
StringBuffer_append(StringBuffer* buffer, size_t offset, const char* source, size_t nchars)
{
	XFDBG("\nbuffer[length:%zu, used:%zu, unused:%zu] source[nchars:%zu]\n",
	      StringBuffer_length(buffer), StringBuffer_used(buffer), StringBuffer_unused(buffer), nchars);

	/* allocate space for additional \0 if input is not \0 terminated */
	if (!is_nullterm(source, nchars))
		nchars += 1;

	if (StringBuffer_make_room(buffer, offset, nchars) == -1)
		return -1;

	memcpy(S_get(buffer->string, offset), source, nchars);
	buffer->string->length = offset + nchars;

	S_nullterm(buffer->string);

	return (ssize_t)nchars;
}

ssize_t
StringBuffer_read(StringBuffer* buffer, size_t offset, int fd, size_t nchars)
{
	// always make room for an additional '\0' terminator
	// FIXME check why post increment is not working with unsigned values
	size_t nalloc_chars = nchars + 1;

	if (StringBuffer_make_room(buffer, offset, nalloc_chars) == -1)
		return -1;

	ssize_t nread = read(fd, S_get(buffer->string, offset), nchars);
	XFLOG("Read %zu (read size %zu) chars into buffer", nread, nchars);

	if (nread > 0)
		buffer->string->length = offset + (size_t)nread;

	if (!S_is_nullterm(buffer->string))
	{
		buffer->string->length += 1;
		S_nullterm(buffer->string);
	}

	return nread;
}

ssize_t
StringBuffer_fdload(StringBuffer* buffer, int fd, size_t chunk_size)
{
	ssize_t total_read = 0;

	while (1)
	{
		ssize_t nread = StringBuffer_fdncat(buffer, fd, chunk_size);
		total_read += nread;

		if (nread == 0)
			break;
		if (nread < 0)
			return nread;
	}
	return total_read;
}

/* -1 on error, >= 0 for count of printed characters */
ssize_t
StringBuffer_vsprintf(StringBuffer* buffer, size_t offset, const char* format, ...)
{
	/* check that offset is within within range */
	if (offset > buffer->length)
		return -1;

	char* string_start = S_get(buffer->string, offset);
	size_t nchars_available = buffer->length - offset;
	size_t nchars_printed;
	va_list ap;

	va_start(ap, format);
	nchars_printed = (size_t)vsnprintf(string_start, nchars_available, format, ap) + 1 /* \0 */;
	va_end(ap);

	/* check if buffer was large enough */
	if (nchars_printed > nchars_available)
	{
		StringBuffer_make_room(buffer, offset, nchars_printed);
		va_start(ap, format);
		string_start = S_get(buffer->string, offset);
		nchars_available = buffer->length - offset;
		vsnprintf(string_start, nchars_available, format, ap);
		va_end(ap);
	}

	buffer->string->length = offset + nchars_printed;

	return (ssize_t)nchars_printed;
}

/* string pointer methods */

StringPointer*
StringPointer_new(const char* data, size_t length)
{
	StringPointer* pointer = malloc(sizeof(StringPointer));

	pointer->value = data;
	pointer->length = length;
	return pointer;
}

void
StringPointer_free(StringPointer* pointer)
{
	free(pointer);
}
