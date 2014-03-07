#include "stddef.h"     /* size_t */
#include <stdlib.h>     /* free */
#include <stdio.h>      /* printf */
#include <string.h>     /* memcpy */
#include "libcx-base/debug.h"

typedef struct buffer_t
{
	char *data;
	size_t length;
} Buffer;

Buffer *
Buffer_new();

void
Buffer_free(Buffer *buf);

void
Buffer_append(Buffer *buf, char* data, size_t length);
