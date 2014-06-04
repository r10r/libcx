#include "../base/test.h"

#include "server_unix.h"
#include "server_tcp.h"

#define CONNECTION_BUFFER_LENGTH 1024

static void
close_connection(Connection* connection)
{
	StringBuffer* buffer = (StringBuffer*)connection->data;

	StringBuffer_free(buffer);
	Connection_close(connection);
}

static void
connection_close(Connection* conn, SendBuffer* buffer)
{
	UNUSED(buffer);
	Connection_close(conn);
}

static void
echo_connection_read(Connection* conn)
{
	CXDBG(conn, "read data");

	if (!conn->data)
		conn->data = StringBuffer_new(CONNECTION_BUFFER_LENGTH);
	StringBuffer* buffer = (StringBuffer*)conn->data;

	/* receive data until buffer is full */
	StringBuffer_ffill(buffer, conn->fd);

	CXFDBG(conn, "buffer status %d", buffer->status);

	switch (buffer->status)
	{
	case STRING_BUFFER_STATUS_OK:
		Connection_send(conn, buffer, NULL);
		conn->data = NULL;
		break;
	case STRING_BUFFER_STATUS_EOF:
		/* todo check if there is any data to send */
		XDBG("received EOF - closing connection");
		Connection_close_read(conn);
		Connection_send(conn, buffer, connection_close);
		break;
	case STRING_BUFFER_STATUS_ERROR_TO_SMALL:
	case STRING_BUFFER_STATUS_ERROR_INVALID_ACCESS:
	case STRING_BUFFER_STATUS_ERROR_INVALID_READ_SIZE:
	{
		CXFDBG(conn, "closing connection because of error :%d", buffer->status);
		close_connection(conn);
		break;
	}
	case STRING_BUFFER_STATUS_ERROR_ERRNO:
	{
		if (buffer->error_errno == EWOULDBLOCK)
		{
			Connection_send(conn, buffer, NULL);
			conn->data = NULL;
		}
		else
		{
			CXFDBG(conn, "closing connection because of error :%d", buffer->status);
			close_connection(conn);
		}
		break;
	}
	}
}

static Connection*
EchoConnection_new()
{
	Connection* connection = Connection_new(NULL, -1);

	connection->f_receive_data_handler = echo_connection_read;
	connection->f_on_write_error = Connection_close;
	connection->data = StringBuffer_new(CONNECTION_BUFFER_LENGTH);
	return connection;
}

static ConnectionWorker*
EchoWorker_new()
{
	ConnectionWorker* worker = ConnectionWorker_new();

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
