#include "../base/test.h"

#include "server_unix.h"
#include "server_tcp.h"
#include "echo_connection.h"
#include "echo_service.h"

// TODO improve connection logging

static void
on_start(Connection* conn)
{
	UNUSED(conn);
	CXLOG(conn, "ON START");
}

static void
on_close(Connection* conn)
{
	UNUSED(conn);
	XDBG("ON CLOSE");
}

static void
on_error(Connection* conn)
{
	UNUSED(conn);
	XDBG("Connection error");
}

static void
on_request(Connection* conn, Request* request)
{
	UNUSED(conn);
	XLOG("ON REQUEST");
	StringBuffer* request_buffer = (StringBuffer*)Request_get_userdata(request);
	XFLOG("request >>>>\n%s\n", StringBuffer_value(request_buffer));
	Request_free(request);
	conn->f_send(conn, Response_new(request_buffer));
}

static ConnectionCallbacks echo_handler = {
	.on_close	= &on_close,
	.on_error	= &on_error,
	.on_request	= &on_request,
	.on_start	= &on_start
};

static void
print_usage(const char* message) __attribute__((noreturn));

static void
print_usage(const char* message)
{
	fprintf(stderr, "Error: %s\n", message);
	fprintf(stderr, "Usage: $0 <unix|tcp> <worker count>\n");
	exit(1);
}

int
main(int argc, char** argv)
{
	if (argc != 3)
		print_usage("Invalid parameter count");

	Server* server = NULL;

	if (strcmp(argv[1], "unix") == 0)
		server = (Server*)UnixServer_new("/tmp/echo.sock");
	else if (strcmp(argv[1], "tcp") == 0)
		server = (Server*)TCPServer_new("0.0.0.0", 6666);
	else
		print_usage("Invalid server type");

	int worker_count = atoi(argv[2]);

	/* userdata */
	RPC_Method rpc_methods[] = { EchoService_methods, RPC_Method_none };
	server->data = rpc_methods;

	ConnectionWorker* worker = ConnectionWorker_new(EchoConnection_new, &echo_handler);

	int i;
	for (i = 0; i < worker_count; i++)
		List_push(server->workers, worker);

	Server_start(server);         // blocks
}
