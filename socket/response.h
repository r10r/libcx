#ifndef _CX_RESPONSE_H
#define _CX_RESPONSE_H

typedef struct cx_response_t Response;

#include "connection.h"

typedef void F_SendFinished (Connection* conn, Response* response);
typedef void F_SendError (Connection* conn, Response* response, int error);
typedef size_t F_DataGet (Response* response, const char**);
typedef bool F_DataAvailable (Response* response);
typedef void F_DataTransmitted (Response* response, size_t ntransmitted);

struct cx_response_t
{
	size_t ntransmitted;
	F_SendFinished* on_finished;
	F_DataAvailable* data_available;
	F_DataGet* data_get;
	F_DataTransmitted* on_data_transmitted;
	F_SendError* on_error;
	void* data;
};

Response*
Response_new(StringBuffer* buffer, F_SendFinished* f_finished);

void
Response_free(Response* response);

#endif
