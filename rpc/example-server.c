#include <jansson.h>

#include <libcx/socket/server_unix.h>
#include <libcx/socket/server_tcp.h>

#include <libcx/socket/ws/ws_connection.h>

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

static void
on_request(Connection* conn, Request* request)
{
	CXDBG(conn, "ON REQUEST");

	json_t* json = RPC_process(EXAMPLE_SERVICE_METHODS, request);

	if (json)
	{
		// FIXME pass function to websockets create to gather data (using the jansson dump callback)
		char* json_string = json_dumps(json, JSON_INDENT(2));
		StringBuffer* buffer  = WebsocketsFrame_create(WS_FRAME_TEXT, json_string, strlen(json_string));
		cx_free(json_string);
		json_decref(json);
		conn->f_send(conn, Response_new(buffer), NULL);
	}

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
