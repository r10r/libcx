#include "base/test.h"
#include "base/base.h"
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
echo_connection_read(Connection* conn)
{
	CXDBG(conn, "read data");
	StringBuffer* buffer = (StringBuffer*)conn->data;

	/* receive data until buffer is full */
	StringBuffer_ffill(buffer, conn->fd);

	CXFDBG(conn, "buffer status %d", buffer->status);
	switch (buffer->status)
	{
	case STRING_BUFFER_STATUS_OK:
		Connection_start_write(conn);
		break;
	case STRING_BUFFER_STATUS_EOF:
		/* todo check if there is any data to send */
		XDBG("received EOF - closing connection");
		Connection_close_read(conn);
		Connection_start_write(conn);
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
			Connection_start_write(conn);
		else
		{
			CXFDBG(conn, "closing connection because of error :%d", buffer->status);
			close_connection(conn);
		}
		break;
	}
	}
}

static void
echo_connection_write(Connection* conn)
{
	CXDBG(conn, "write data");
	StringBuffer* buffer = (StringBuffer*)conn->data;
	size_t nused = StringBuffer_used(buffer);

	if (nused == 0)
	{
		CXDBG(conn, "no more data available for writing");

		if (buffer->status == STRING_BUFFER_STATUS_EOF)
			close_connection(conn);
		else
			Connection_stop_write(conn);
	}
	else
	{
		ssize_t nwritten = write(conn->fd, StringBuffer_value(buffer), nused);

		if (nwritten == -1)
		{
			// we should not receive EAGAIN here ?
			assert(errno != EAGAIN);
			// FIXME errno:32:[Broken pipe] - Connection[12] - Failed to write data
			CXERRNO(conn, "Failed to write data");
			// when writing fails shutdown connection, because buffer is not shifted any longer
			Connection_close(conn);
		}
		else
		{
			CXFDBG(conn, "send %zu bytes (remaining %zu)", nwritten, StringBuffer_used(buffer));
			StringBuffer_shift(buffer, (size_t)nwritten);
		}
	}
}

static Connection*
EchoConnection_new()
{
	Connection* connection = Connection_new(NULL, -1);

	connection->f_receive_data_handler = echo_connection_read;
	connection->f_send_data_handler = echo_connection_write;
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
