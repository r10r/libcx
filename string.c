#include "string.h"

String*
String_init(const char *value, size_t length)
{
	String *s;
	size_t size = sizeof(String) + sizeof(char) * length;

	if (value)
	{
		s = malloc(size);
		memcpy(&s->value[0], value, length);
		s->length = length;
	}
	else
	{
		s = calloc(1, size);
		s->length = 0;
	}
	return s;
}

StringBuffer*
StringBuffer_new(size_t length)
{
	StringBuffer *buf = malloc(sizeof(StringBuffer));

	buf->length = length;
	buf->string = String_init(NULL, length);
	return buf;
}

/*
 * @return 0 if room is available -1 else
 */
int
StringBuffer_make_room(StringBuffer *buffer, size_t nchars)
{
	size_t nunused = SBuf_unused(buffer);

	/* more unused bytes available than requested */
	if (nunused >= nchars)
		return 0;

	/* allocate missing bytes */
	size_t nalloc = nchars - nunused;
	if (nunused + nalloc > SIZE_MAX)
		return -1;

	String *new_string = S_realloc(buffer->string, nalloc);
	if (new_string == NULL)
		return -1;

	buffer->string = new_string;
	buffer->length += nalloc;
	return 0;
}

ssize_t
StringBuffer_ncopy(StringBuffer *buffer, ssize_t index, const char* source, size_t nchars)
{
	if (StringBuffer_make_room(buffer, nchars) != 0)
		return -1;

	memcpy(&S_at(buffer->string, index), source, nchars);
	buffer->string->length += nchars;
	return (ssize_t)nchars;
}

ssize_t
StringBuffer_nread(StringBuffer *buffer, int index, int fd, size_t nchars)
{
	if (StringBuffer_make_room(buffer, nchars) != 0)
		return -1;

	ssize_t nread = read(fd, &S_at(buffer->string, index), nchars);
	if (nread > 0)
		buffer->string->length += (size_t)nread;
	return nread;
}
