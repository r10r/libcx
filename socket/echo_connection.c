#include "echo_connection.h"

static void
close_connection(Connection* conn)
{
	StringBuffer_free((StringBuffer*)conn->f_get_userdata(conn));
	conn->f_close(conn);
}

static void
handle_error(Connection* conn)
{
	CXDBG(conn, "closing connection because of an error");
	Connection_callback(conn, on_error);
	close_connection(conn);
}

static void
handle_request(Connection* conn)
{
	StringBuffer* buffer = (StringBuffer*)conn->f_get_userdata(conn);

	conn->f_set_userdata(conn, NULL);

	size_t num_bytes_received = StringBuffer_used(buffer);
	CXFDBG(conn, "processing received data (%zu bytes)", num_bytes_received);

	if (num_bytes_received > 0)
	{
		if (conn->callbacks->on_request)
			conn->callbacks->on_request(conn, Request_new(buffer));
	}
	else
	{
		StringBuffer_free(buffer);
	}
}

static void
echo_connection_read(Connection* conn, int fd)
{
	CXDBG(conn, "read data");

	StringBuffer* buffer = (StringBuffer*)conn->f_get_userdata(conn);

	if (!buffer)
	{
		buffer = StringBuffer_new(CONNECTION_BUFFER_LENGTH);
		conn->f_set_userdata(conn, buffer);
	}

	/* read data until buffer is full */
	StringBuffer_ffill(buffer, fd);

	CXFDBG(conn, "buffer status %d", buffer->status);

	switch (buffer->status)
	{
	case STRING_BUFFER_STATUS_OK:
		handle_request(conn);
		break;
	case STRING_BUFFER_STATUS_EOF:
		CXDBG(conn, "received EOF - closing connection");
		conn->f_close_read(conn);
		handle_request(conn);
		close_connection(conn); // FIXME proper close connection handling
		break;
	case STRING_BUFFER_STATUS_ERROR_TO_SMALL:
	case STRING_BUFFER_STATUS_ERROR_INVALID_ACCESS:
	case STRING_BUFFER_STATUS_ERROR_INVALID_READ_SIZE:
	{
		handle_error(conn); // FIXME proper close connection handling
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

Connection*
EchoConnection_new(ConnectionCallbacks* callbacks)
{
	Connection* connection = Connection_new(callbacks);

	connection->f_receive = echo_connection_read;
	return connection;
}
