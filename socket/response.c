#include "response.h"

typedef struct buffer_response_t
{
	Response response;
	StringBuffer* buffer;
	size_t transmitted;
} BufferedResponse;

static size_t
data_get(Response* response, const char** data)
{
	BufferedResponse* buffer_response = (BufferedResponse*)response;
	size_t bytes_not_transmitted = StringBuffer_used(buffer_response->buffer) - buffer_response->transmitted;

	assert(bytes_not_transmitted >= 0);

	if (bytes_not_transmitted > 0)
	{
		*data = StringBuffer_at(buffer_response->buffer, buffer_response->transmitted);
	}
	else if (bytes_not_transmitted == 0)
	{
		*data = NULL;
	}
	return bytes_not_transmitted;
}

static bool
data_available(Response* response)
{
	BufferedResponse* buffer_response = (BufferedResponse*)response;

	return StringBuffer_used(buffer_response->buffer) > buffer_response->transmitted;
}

static void
data_transmitted(Response* response, size_t transmitted)
{
	((BufferedResponse*)response)->transmitted += transmitted;
}

static void
on_error(Connection* conn, Response* response, int error)
{
	UNUSED(conn);
	UNUSED(response);
	XFERR("Error[%d] while sending response", error);
}

Response*
Response_new(StringBuffer* buffer, F_SendFinished* f_finished)
{
	Response* response = cx_alloc(sizeof(BufferedResponse));

	((BufferedResponse*)response)->buffer = buffer;

	response->data_get = &data_get;
	response->on_data_transmitted = &data_transmitted;
	response->on_error = &on_error;
	response->on_finished = f_finished;
	response->data_available = &data_available;

	return response;
}

void
Response_free(Response* response)
{
	StringBuffer_free(((BufferedResponse*)response)->buffer);
	cx_free(response);
}
