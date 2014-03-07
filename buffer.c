#include "buffer.h"

void
Buffer_free(Buffer *buf)
{
	free(buf->data);
	free(buf);
}

Buffer *
Buffer_new()
{
	Buffer *buf = malloc(sizeof(Buffer));
	buf->length = 0;
	buf->data = NULL;
	return buf;
}

void
Buffer_append(Buffer *buf, char* data, size_t length)
{
	int offset = buf->length;

	XLOG("Append data (size %d) to buffer (size %d)\n",
	       (int)length, (int)buf->length);
	buf->length += length;

	if (offset == 0)
		buf->data = malloc(sizeof(char) * buf->length);
	else
		buf->data = realloc(buf->data, sizeof(char) * buf->length);
	memcpy(buf->data + offset, data, length);
}
