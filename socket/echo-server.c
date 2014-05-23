#include "base/test.h"
#include "base/base.h"
#include "server_unix.h"
#include "server_tcp.h"

#define CONNECTION_BUFFER_LENGTH 1024

static ssize_t
echo_connection_data_handler(Connection* connection)
{
	StringBuffer* buffer = (StringBuffer*)connection->data;

	return StringBuffer_fdxload(buffer, connection->fd, READ_MAX, 0);
}

static Connection*
echo_connection_handler(Connection* connection, ConnectionEvent event)
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
		XDBG("close read");
		StringBuffer_free((StringBuffer*)connection->data);
		Connection_close(connection);
		break;
	}
	case CONNECTION_EVENT_ERRNO:
		perror("failed to accept connection");
		break;
	case CONNECTION_EVENT_ERROR_WRITE:
		break;
	}
	return connection;
}

static Connection*
EchoConnection_new()
{
	Connection* connection = Connection_new(NULL, -1);

	connection->f_data_handler = echo_connection_data_handler;
	connection->f_handler = echo_connection_handler;
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

	int i;
	for (i = 0; i < worker_count; i++)
		List_push(server->workers, EchoWorker_new());

	Server_start(server);         // blocks
}
