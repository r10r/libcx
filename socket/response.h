#ifndef _CX_RESPONSE_H
#define _CX_RESPONSE_H

#include <stdbool.h> /* bool */
#include <stddef.h>

#include "../string/string_buffer.h"

typedef struct cx_response_t Response;

typedef size_t F_DataGet (Response* response, const char**);
typedef bool F_DataAvailable (Response* response);
typedef void F_DataTransmitted (Response* response, size_t ntransmitted);

struct cx_response_t
{
	size_t ntransmitted;
	F_DataAvailable* data_available;
	F_DataGet* data_get;
	F_DataTransmitted* on_data_transmitted;
	void* data;
};

/* FIXME move to StringBufferResponse (implementation specific) */
Response*
Response_new(StringBuffer* buffer);

void
Response_free(Response* response);

#endif
