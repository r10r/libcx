#include "base/test.h"
#include "base/base.h"
#include "socket/server_unix.h"
#include "socket/server_tcp.h"

#include "rpc/jsrpc.h"
#include "mpd_service.h"

// TODO make this a generic RPC server

typedef struct rpc_server_t
{
	Server server;
	RPC_Method* methods;
} RPC_Server;

static ssize_t
rpc_connection_data_handler(Connection* connection)
{
	RPC_RequestList* pipeline = (RPC_RequestList*)connection->data;

	return StringBuffer_fdcat(pipeline->request_buffer, connection->fd);
}

static Connection*
rpc_connection_handler(Connection* connection, ConnectionEvent event)
{
	switch (event)
	{
	case CONNECTION_EVENT_DATA:
	{
		break;
	}
	case CONNECTION_EVENT_CLOSE_READ:
	{
		XDBG("processing request");

		RPC_RequestList* request_list = (RPC_RequestList*)connection->data;
		RPC_Method* service_methods = ((RPC_Server*)connection->worker->server)->methods;
		RPC_RequestList_process(request_list, service_methods);
		Connection_send_buffer(connection, request_list->response_buffer);
		RPC_RequestList_free(request_list);
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
RPCConnection_new()
{
	Connection* connection = Connection_new(NULL, -1);

	connection->f_data_handler = rpc_connection_data_handler;
	connection->f_handler = rpc_connection_handler;
	connection->data = RPC_RequestList_new();
	return connection;
}

static UnixWorker*
RPCWorker_new()
{
	UnixWorker* worker = UnixWorker_new();

	worker->f_create_connection = RPCConnection_new;
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
		UnixServer_init((Server*)server, "/tmp/mpd.sock");
	else if (strcmp(argv[1], "tcp") == 0)
		TCPServer_init((Server*)server, "127.0.0.1", 6666);
	else
		print_usage("Invalid server type");

	RPC_Method mpd_methods[] = { MusicPlayerDaemon_methods, RPC_Method_none };
	server->methods = &mpd_methods[0];

	printf("Registered RPC methods:\n");
	int i;
	for (i = 0; mpd_methods[i].name != NULL; i++)
		RPC_Method_log(&mpd_methods[i]);

	List_push(((Server*)server)->workers, RPCWorker_new());
	List_push(((Server*)server)->workers, RPCWorker_new());
	List_push(((Server*)server)->workers, RPCWorker_new());
	List_push(((Server*)server)->workers, RPCWorker_new());

	Server_start((Server*)server);         // blocks
}
