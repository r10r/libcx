#include "socket/server_unix.h"
#include "socket/server_tcp.h"

#include "socket/ws/ws_connection.h"

#include "rpc_json.h"
#include "rpc_example_service.h"

static void
print_usage(const char* message) __attribute__((noreturn));

static void
print_usage(const char* message)
{
	fprintf(stderr, "Error: %s\n", message);
	fprintf(stderr, "Usage: $0 <port>\n");
	exit(1);
}

static void
on_error(Connection* conn)
{
	UNUSED(conn);
	CXDBG(conn, "Connection error");
}

#include <jansson.h>

static void
on_request(Connection* conn, Request* request)
{
	CXDBG(conn, "ON REQUEST");
	char* payload = NULL;
	size_t payload_len = request->f_get_payload(request, &payload);

	RPC_Request rpc_request;
	int status = Request_from_json(&rpc_request, payload, payload_len);
	if (status == 0)
	{
		status = Service_call(EXAMPLE_SERVICE_METHODS, &rpc_request);

		// FIXME What should a request for a method that returns nothing (void method) return ?
		// (null, true) ?

		/* ignore notifications */
		if (rpc_request.id_type == RPC_ID_NONE)
		{
			XLOG("RPC request is a notification");
		}
		else
		{
			if (status == -1) /* error */
			{
				XFERR("RPC execution error: %d", rpc_request.error);
				/* TODO send error */
			}
			else if (status == 0) /* success no return value (empty array | empty object ?) */
			{
				XFLOG("RPC method(%s) executed (void method)", rpc_request.method_name);
				// TODO send response with result set to null ? (result is required on success !!)
			}
			else if (status == 1) /* success with return value */
			{
				XFLOG("RPC method(%s) executed (with return value)", rpc_request.method_name);
				json_t* json = rpc_request.result.f_to_json(rpc_request.result.value.object);
				char* json_string = json_dumps(json, JSON_INDENT(2));
				StringBuffer* buffer = WebsocketsFrame_create(WS_FRAME_TEXT, json_string, strlen(json_string));
				cx_free(json_string);
				json_decref(json);
				conn->f_send(conn, Response_new(buffer), NULL);
			}
		}
	}
	else
	{
		XFERR("Error processing JSON RPC 2.0 request: RPC Error %d", rpc_request.error)
	}

	if (rpc_request.result.f_free)
		rpc_request.result.f_free(rpc_request.result.value.object);

	if (rpc_request.f_free)
		rpc_request.f_free(&rpc_request);

	// call method

	Request_free(request);
//	StringBuffer* buffer = WebsocketsFrame_create(WS_FRAME_TEXT, payload, payload_len);
//	conn->f_send(conn, Response_new(buffer), NULL);
}

static void
on_start(Connection* conn)
{
	UNUSED(conn);
	CXDBG(conn, "ON START");
}

static void
on_close(Connection* conn)
{
	UNUSED(conn);
	CXDBG(conn, "ON CLOSE");
}

static ConnectionCallbacks ws_echo_handler = {
	.on_close       = &on_close,
	.on_error       = &on_error,
	.on_request     = &on_request,
	.on_start       = &on_start
};

int
main(int argc, char** argv)
{
	if (argc != 2)
		print_usage("Invalid parameter count");

	Server* server = NULL;

//	if (strcmp(argv[1], "unix") == 0)
//		server = (Server*)UnixServer_new("/tmp/echo.sock");
//	else if (strcmp(argv[1], "tcp") == 0)
	server = (Server*)TCPServer_new("0.0.0.0", (uint16_t)atoi(argv[1]));
//	else
//		print_usage("Invalid server type");

	List_push(server->workers, ConnectionWorker_new(WebsocketsConnection_new, &ws_echo_handler));
	List_push(server->workers, ConnectionWorker_new(WebsocketsConnection_new, &ws_echo_handler));
	List_push(server->workers, ConnectionWorker_new(WebsocketsConnection_new, &ws_echo_handler));
	List_push(server->workers, ConnectionWorker_new(WebsocketsConnection_new, &ws_echo_handler));

	Server_start(server);         // blocks
}
