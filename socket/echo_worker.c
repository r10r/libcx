#include "echo_worker.h"

static void
close_connection(Connection* connection)
{
	StringBuffer* buffer = (StringBuffer*)connection->data;

	StringBuffer_free(buffer);
	Connection_close(connection);
}

static void
handle_error(Connection* conn)
{
	CXDBG(conn, "closing connection because of error");

	if (conn->connection_callbacks->on_error)
		conn->connection_callbacks->on_error(conn);

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
		if (conn->connection_callbacks->on_request)
			conn->connection_callbacks->on_request(conn, Request_new(buffer));
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
		// FIXME close connection after
		// async request processing close_connection(conn);
		handle_request(conn);
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

static Connection*
EchoConnection_new(ConnectionWorker *worker)
{
	Connection* connection = Connection_new(NULL, -1);

	connection->f_receive_data_handler = echo_connection_read;
	connection->f_on_write_error = Connection_close;
	connection->data = StringBuffer_new(CONNECTION_BUFFER_LENGTH);

	connection->connection_callbacks = worker->connection_callbacks;

	return connection;
}

ConnectionWorker*
EchoWorker_new(ConnectionCallbacks *connection_callbacks)
{
	ConnectionWorker* worker = ConnectionWorker_new(connection_callbacks);

	worker->f_create_connection = EchoConnection_new;
	return worker;
}
