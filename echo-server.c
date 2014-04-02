#include "libcx-base/test.h"
#include "libcx-base/base.h"
#include "unix_server.h"

#define CONNECTION_BUFFER_LENGTH 1024

static ssize_t
echo_connection_data_handler(Connection *connection)
{
	StringBuffer *buffer = (StringBuffer*)connection->data;

	return StringBuffer_read(buffer, 0, connection->fd, buffer->length);
}

static Connection*
echo_connection_handler(Connection *connection, ConnectionEvent event)
{
	switch (event)
	{
	case CONNECTION_EVENT_DATA:
	{
		StringBuffer *buffer = (StringBuffer*)connection->data;
		Connection_send(connection, buffer->string->value, buffer->string->length);
		break;
	}
	case CONNECTION_EVENT_CLOSE_READ:
	{
		XDBG("close read");
		const char *byebye = "bye bye\n";
		Connection_send(connection, byebye, strlen(byebye));
		StringBuffer_free((StringBuffer*)connection->data);
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
	Connection *connection = Connection_new(NULL, -1);

	connection->f_data_handler = echo_connection_data_handler;
	connection->f_handler = echo_connection_handler;
	connection->data = StringBuffer_new(CONNECTION_BUFFER_LENGTH);
	return connection;
}

static UnixWorker*
EchoWorker_new()
{
	UnixWorker *worker = UnixWorker_new();

	worker->f_create_connection = EchoConnection_new;
	return worker;
}

int
main(int argc, char** argv)
{
	int worker_count = (argc == 2) ? atoi(argv[1]) : 4;

	Server *server = (Server*)UnixServer_new("/tmp/echo.sock");

	int i;

	for (i = 0; i < worker_count; i++)
		List_push(server->workers, EchoWorker_new());

	int ret = Server_start(server);         // blocks
}
