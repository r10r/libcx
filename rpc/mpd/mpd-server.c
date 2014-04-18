#include "base/test.h"
#include "base/base.h"
#include "socket/server_unix.h"
#include "socket/server_tcp.h"

#include "rpc/jsrpc.h"
#include "mpd_service.h"

#define CONNECTION_BUFFER_LENGTH 1024

// {"jsonrpc": "2.0", "id": 66, "method": "foobar"}

typedef struct rpc_server_t
{
	Server server;
	RPC_Method* method;
} RPC_Server;


static ssize_t
rpc_connection_data_handler(Connection* connection)
{
	StringBuffer* buffer = (StringBuffer*)connection->data;

	return StringBuffer_read(buffer, 0, connection->fd, buffer->length);
}

static Connection*
rpc_connection_handler(Connection* connection, ConnectionEvent event)
{
	switch (event)
	{
	case CONNECTION_EVENT_DATA:
	{
		StringBuffer* buffer = (StringBuffer*)connection->data;
		Connection_send(connection, buffer->string->value, buffer->string->length);
		break;
	}
	case CONNECTION_EVENT_CLOSE_READ:
	{
		XDBG("processing request");

		StringBuffer* buffer = (StringBuffer*)connection->data;
		StringBuffer_catn(buffer, ""); // add \0 terminator to buffer
//		Connection_send(connection, byebye, strlen(byebye));
		dispatch_request(((RPC_Server*)connection->worker->server)->method, buffer->string->value);
		StringBuffer_free(buffer);
		Connection_close(connection);
		break;
	}
	case CONNECTION_EVENT_ERRNO:
	case CONNECTION_EVENT_ERROR_WRITE:
		break;
	}
	return connection;
}

static Connection*
EchoConnection_new()
{
	Connection* connection = Connection_new(NULL, -1);

	connection->f_data_handler = rpc_connection_data_handler;
	connection->f_handler = rpc_connection_handler;
	connection->data = StringBuffer_new(CONNECTION_BUFFER_LENGTH);
	return connection;
}

static UnixWorker*
EchoWorker_new()
{
	UnixWorker* worker = UnixWorker_new();

	worker->f_create_connection = EchoConnection_new;
	return worker;
}

static void
print_usage(const char* message) __attribute__((noreturn));

static void
print_usage(const char* message)
{
	fprintf(stderr, "Error: %s\n", message);
	fprintf(stderr, "Usage: $0 <unix|tcp>\n");
	exit(1);
}

int
main(int argc, char** argv)
{
	if (argc != 2)
		print_usage("Invalid parameter count");

	RPC_Server* server = malloc(sizeof(RPC_Server));

	if (strcmp(argv[1], "unix") == 0)
		UnixServer_init((Server*)server, "/tmp/echo.sock");
	else if (strcmp(argv[1], "tcp") == 0)
		TCPServer_init((Server*)server, "127.0.0.1", 6666);
	else
		print_usage("Invalid server type");

	RPC_Method mpd_methods[] = { RPC_methods(MusicPlayerDaemon), RPC_null };
	server->method = &mpd_methods[0];

	printf("Registered RPC methods:\n");
	int i;
	for (i = 0; mpd_methods[i].name != NULL; i++)
		RPC_Method_log(&mpd_methods[i]);

	List_push(((Server*)server)->workers, EchoWorker_new());

	Server_start((Server*)server);         // blocks
}
