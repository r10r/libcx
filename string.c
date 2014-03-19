#include "string.h"

String*
String_init(const char *value, size_t length)
{
	if (length > STRING_MAX_LENGTH)
		return NULL;

	String *s = S_alloc(length);
	s->length = 0;

	if (value)
	{
		memcpy(&s->value[0], value, length);
		s->length = length;
	}
	return s;
}

StringBuffer*
StringBuffer_new(size_t length)
{
	if (length > STRING_MAX_LENGTH)
		return NULL;

	StringBuffer *buf = malloc(sizeof(StringBuffer));

	buf->length = length;
	buf->string = String_init(NULL, length);
	return buf;
}

void
StringBuffer_free(StringBuffer *buffer)
{
	if (buffer)
	{
		S_free(buffer->string);
		free(buffer);
	}
}

/*
 * @return 0 if room is available -1 else
 */
int
StringBuffer_make_room(StringBuffer *buffer, size_t offset, size_t nlength_requested)
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

	String *new_string = S_realloc(buffer->string, new_length);
	if (new_string == NULL)
		return -1;

	buffer->string = new_string;
	buffer->length = new_length;
	return 0;
}

ssize_t
StringBuffer_ncopy(StringBuffer *buffer, size_t offset, const char* source, size_t nchars)
{
	if (StringBuffer_make_room(buffer, offset, nchars) != 0)
		return -1;

	memcpy(&S_get(buffer->string, offset), source, nchars);
	buffer->string->length = offset + nchars;
	return (ssize_t)nchars;
}

ssize_t
StringBuffer_nread(StringBuffer *buffer, size_t offset, int fd, size_t nchars)
{
	if (StringBuffer_make_room(buffer, offset, nchars) != 0)
		return -1;

	ssize_t nread = read(fd, &S_get(buffer->string, offset), nchars);
	if (nread > 0)
		buffer->string->length = offset + (size_t)nread;
	return nread;
}
