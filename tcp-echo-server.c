#include "libcx-base/test.h"
#include "libcx-base/base.h"
#include "tcp_server.h"

#define CONNECTION_BUFFER_LENGTH 1024

static ssize_t
echo_connection_data_handler(Connection *connection)
{
	StringBuffer *buffer = (StringBuffer*)connection->data;

	// fill buffer
	return StringBuffer_read(buffer, 0, connection->fd, buffer->length);
}

static Connection*
echo_connection_handler(Connection *connection, ConnectionEvent event)
{
	switch (event)
	{
	case CONNECTION_EVENT_ACCEPTED:
		XDBG("connection accepted");
		connection->data = StringBuffer_new(CONNECTION_BUFFER_LENGTH);
		break;
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
	case CONNECTION_EVENT_RECEIVE_TIMEOUT:
	case CONNECTION_EVENT_ERRNO:
	case CONNECTION_EVENT_ERROR_WRITE:
		break;
	}
	return connection;
}

int
main(int argc, char** argv)
{
	Server *server = (Server*)TCPServer_new("0.0.0.0", 6666);

	server->worker_count = (argc == 2) ? atoi(argv[1]) : 4;
	server->f_connection_handler = echo_connection_handler;
	server->f_connection_data_handler = echo_connection_data_handler;
	int ret = Server_start(server); // blocks
}
