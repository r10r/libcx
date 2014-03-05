#include "buffer.h"

void
buffer_free(Buffer *buf)
{
	free(buf->data);
	buf->length = 0;
}

void
buffer_new(Buffer *buf)
{
	buf->length = 0;
	buf->data = NULL;
}

void
buffer_append(Buffer *buf, char* data, size_t data_length)
{
	int offset = buf->length;

	printf("Append data (size %d) to buffer (size %d)\n",
	       (int)data_length, (int)buf->length);
	buf->length += data_length;

	if (offset == 0)
		buf->data = malloc(sizeof(char) * buf->length);
	else
		buf->data = realloc(buf->data, sizeof(char) * buf->length);
	memcpy(buf->data + offset, data, data_length);
}
