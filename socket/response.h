#ifndef _CX_RESPONSE_H
#define _CX_RESPONSE_H

#include <stdbool.h> /* bool */
#include <stddef.h>

#include "../string/string_buffer.h"

typedef struct cx_response_t Response;

typedef size_t F_DataGet (Response* response, const char**);
typedef bool F_DataAvailable (Response* response);
typedef void F_DataTransmitted (Response* response, size_t ntransmitted);
/* FIXME connection is void so we do not have to import the connection */
typedef void F_ResponseFinished (Response* response, void* connection);

struct cx_response_t
{
	size_t ntransmitted;
	F_DataAvailable* f_data_available;
	F_DataGet* f_data_get;
	F_DataTransmitted* on_data_transmitted;
	F_ResponseFinished* on_finished;

	void* data;
};

/* FIXME move to StringBufferResponse (implementation specific) */
Response*
Response_new(StringBuffer* buffer);

void
Response_free(Response* response);

void
Response_set_data(Response* response, void* data);

void*
Response_get_data(Response* response);

#endif
