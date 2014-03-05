#include "stddef.h"     /* size_t */
#include <stdlib.h>     /* free */
#include <stdio.h>      /* printf */
#include <string.h>     /* memcpy */

typedef struct buffer_t
{
	char *data;
	size_t length;
} Buffer;

void
buffer_free(Buffer *buf);

void
buffer_new(Buffer *buf);

void
buffer_append(Buffer *buf, char* data, size_t data_length);
