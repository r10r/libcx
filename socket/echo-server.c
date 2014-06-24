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

//
//static void
//connection_close(Connection* conn, SendBuffer* buffer)
//{
//	UNUSED(buffer);
//	Connection_close(conn);
//}

static void
handle_error(Connection* conn)
{
	CXDBG(conn, "closing connection because of error");

	if (conn->handler.on_error)
		conn->handler.on_error(conn);

	// TODO allow handler to disable connection close ?
	// FIXME make this async
	close_connection(conn);
}

static void
handle_request(Connection* conn)
{
	StringBuffer* buffer = (StringBuffer*)conn->data;

	conn->data = NULL;

	size_t num_bytes_received = StringBuffer_used(buffer);
	CXFDBG(conn, "processing received data (%zu bytes)", num_bytes_received);

	if (num_bytes_received > 0)
	{
		if (conn->handler.on_request)
			conn->handler.on_request(conn, Request_new(buffer));
	}
	else
	{
		StringBuffer_free(buffer);
	}
}

static void
echo_connection_read(Connection* conn)
{
	CXDBG(conn, "read data");

	if (!conn->data)
		conn->data = StringBuffer_new(CONNECTION_BUFFER_LENGTH);

	StringBuffer* buffer = (StringBuffer*)conn->data;

	/* read data until buffer is full */
	StringBuffer_ffill(buffer, conn->fd);

	CXFDBG(conn, "buffer status %d", buffer->status);

	switch (buffer->status)
	{
	case STRING_BUFFER_STATUS_OK:
		handle_request(conn);
		break;
	case STRING_BUFFER_STATUS_EOF:
		XDBG("received EOF - closing connection");
		Connection_close_read(conn);
		handle_request(conn); // FIXME close connection close_connection(conn);
		close_connection(conn);
		break;
	case STRING_BUFFER_STATUS_ERROR_TO_SMALL:
	case STRING_BUFFER_STATUS_ERROR_INVALID_ACCESS:
	case STRING_BUFFER_STATUS_ERROR_INVALID_READ_SIZE:
	{
		handle_error(conn);
		break;
	}
	case STRING_BUFFER_STATUS_ERROR_ERRNO:
	{
		if (buffer->error_errno == EWOULDBLOCK)
		{
			handle_request(conn);
		}
		else
			handle_error(conn);
		break;
	}
	}
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
	UNUSED(conn);
	XLOG("ON REQUEST");
	StringBuffer* data = (StringBuffer*)request->data;
	XFLOG("request >>>>\n%s\n", StringBuffer_value(data));
	// TODO free request here (attach to response instead ?)
	Request_free(request);

	Connection_send(conn, Response_new(data, NULL));
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

static Connection*
EchoConnection_new()
{
	Connection* connection = Connection_new(NULL, -1);

	connection->f_receive_data_handler = echo_connection_read;
	connection->f_on_write_error = Connection_close;
	connection->data = StringBuffer_new(CONNECTION_BUFFER_LENGTH);

	connection->handler.on_error = &on_error;
	connection->handler.on_request = &on_request;
	connection->handler.on_start = &on_start;
	connection->handler.on_close = &on_close;

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
