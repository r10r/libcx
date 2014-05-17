#include "rpc_server.h"

static void
on_response_send(Connection* conn, Response* response)
{
	UNUSED(conn);
	XFLOG("RESPONSE SEND %p", (void*)response);
}

#define RPC_RESPONSE_EMPTY_BATCH_LENGTH 2 /* [] */

// FIXME make it protocol independent (websockets);
void
rpc_request_callback(Connection* conn, Request* request)
{
	UNUSED(conn);
	XLOG("ON REQUEST");

	WebsocketsFrame* frame = (WebsocketsFrame*)request->data;
	RPC_RequestList* request_list = RPC_RequestList_new(request, frame->payload_raw, RPC_SERVER_BUFFER_SIZE);

	// process request
	// TODO do that async
	RPC_Method* service_methods = ((RPC_Server*)conn->worker->server)->methods;
	RPC_RequestList_process(request_list, service_methods);

	// create response frame (FIXME don't duplicate response data ? (separate header from body ?))
	if (StringBuffer_used(request_list->response_buffer) > RPC_RESPONSE_EMPTY_BATCH_LENGTH)
	{
		StringBuffer* response_frame = WebsocketsFrame_create(WS_FRAME_TEXT,
								      StringBuffer_value(request_list->response_buffer), StringBuffer_used(request_list->response_buffer));
		RPC_RequestList_free(request_list);
		// send response async
		Response* response = Response_new(response_frame, on_response_send);
		Connection_send(conn, response);
	}
	else
	{
		RPC_RequestList_free(request_list);
	}
}
