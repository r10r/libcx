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

	json_t* json = RPC_process(EXAMPLE_SERVICE_METHODS, payload, payload_len);

	StringBuffer* buffer = NULL;
	if (!json)
	{
		XERR("An internal error occured while generating the response");
		buffer = WebsocketsFrame_create(WS_FRAME_TEXT, JSONRPC_INTERNAL_ERROR, strlen(JSONRPC_INTERNAL_ERROR));
	}
	else
	{
		// FIXME pass function to websockets create to gather data (using the jansson write callback)
		char* json_string = json_dumps(json, JSON_INDENT(2));
		buffer = WebsocketsFrame_create(WS_FRAME_TEXT, json_string, strlen(json_string));
		cx_free(json_string);
		json_decref(json);
	}

	conn->f_send(conn, Response_new(buffer), NULL);
	Request_free(request);
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
