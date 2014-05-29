#include "connection.h"

Connection*
Connection_new(Worker* worker, int fd)
{
	Connection* connection = cx_alloc(sizeof(Connection));

	Connection_init(connection, worker, fd);
	return connection;
}

static void
send_buffer_node_free(void* data)
{
	SendBuffer_free((SendBuffer*)data);
}

void
Connection_init(Connection* connection, Worker* worker, int fd)
{
	connection->worker = worker;
	connection->fd = fd;
	connection->send_buffers = List_new();
	connection->send_buffers->f_node_data_free = send_buffer_node_free;
}

static void
receive_data_callback(ev_loop* loop, ev_io* w, int revents)
{
	UNUSED(loop);

	Connection* conn = container_of(w, Connection, receive_data_watcher);

	assert(revents & EV_READ);  /* application bug */

	if (conn->f_receive_data_handler)
		conn->f_receive_data_handler(conn);
}

static void
send_data_callback(ev_loop* loop, ev_io* w, int revents)
{
	UNUSED(loop);

	Connection* conn = container_of(w, Connection, send_data_watcher);

	assert(revents & EV_WRITE);  /* application bug */

	if (conn->f_send_data_handler)
		conn->f_send_data_handler(conn);
}

void
Connection_free(Connection* conn)
{
	List_free(conn->send_buffers);
	cx_free(conn);
}

void
Connection_start(Connection* conn)
{
	unblock(conn->fd);
	ev_io_init(&conn->receive_data_watcher, receive_data_callback, conn->fd, EV_READ);
	ev_io_init(&conn->send_data_watcher, send_data_callback, conn->fd, EV_WRITE);
	ev_io_start(conn->worker->loop, &conn->receive_data_watcher);
}

void
Connection_close_read(Connection* conn)
{
	if (shutdown(conn->fd, SHUT_RD) == -1)
	{
		// remote end has already closed writing
		// ignore this only if EOF has been received previously ?
//		if (errno != ENOTCONN)
		CXERRNO(conn, "failed to shutdown reading end of connection");
	}

	ev_io_stop(conn->worker->loop, &conn->receive_data_watcher);
}

void
Connection_close_write(Connection* conn)
{
	if (shutdown(conn->fd, SHUT_WR) == -1)
		CXERRNO(conn, "failed to shutdown writing end of connection");

	ev_io_stop(conn->worker->loop, &conn->send_data_watcher);
}

void
Connection_close(Connection* connection)
{
	XFDBG("Closing connection[%d]", connection->fd);
	Connection_close_read(connection);
	Connection_close_write(connection);
	Connection_free(connection);
}

void
Connection_send(Connection* conn, StringBuffer* buf, F_SendFinished* f_send_finished)
{
#ifdef _CX_DEBUG
	StringBuffer_print_bytes_hex(buf, 16, "send buffer");
#endif
	SendBuffer* unit = SendBuffer_new(buf, f_send_finished);
	List_push(conn->send_buffers, unit);
	Connection_start_write(conn);
}

//#define WRITE_SIZE SSIZE_MAX

/*
 * - shift what's written ?, or maintain pointer ?
 * - what if output buffer is appended ?
 * - what if data length is really big ?
 *
 * the buffer that is send must be detached !!!!
 * (it can be pooled for performance reasons / to avoid memory fragmentation)
 */

/*
   // @from write function documentation
   //	This function returns the number of bytes transmitted, or -1 on failure. If the socket is nonblocking, then
   //	 send (like write) can return after sending just part of the data. , for information about nonblocking mode.
   //
   //	Note, however, that a successful return value merely indicates that the message has been sent without
   //	 error, not necessarily that it has been received without error.
 */
void
Connection_send_blocking(Connection* c, const char* data, size_t length)
{
	if (length == 0)
	{
		XWARN("Attempting to send data with length 0");
		return;
	}

	/* FIXME this is inefficient brute force sending using busy waiting - use the event loop instead */
	size_t remaining = length;
	size_t processed = 0;
	ssize_t nsend = 0;

	while (remaining > 0)
	{
		nsend = send(c->fd, data + processed, remaining, 0);

		if (nsend == -1)
		{
			if (errno != EWOULDBLOCK || errno != EAGAIN)
				break;
		}
		else
		{
			XFDBG("Sent bytes (%zu of %zu)", nsend, length);
			remaining -= (size_t)nsend;
			processed += (size_t)nsend;
		}
	}

	XDBG("send finished");

	if (c->f_send_data_handler)
		c->f_send_data_handler(c);
}

SendBuffer*
SendBuffer_new(StringBuffer* buffer, F_SendFinished* f_finished)
{
	SendBuffer* unit = cx_alloc(sizeof(SendBuffer));

	unit->buffer = buffer;
	unit->f_send_finished = f_finished;
	return unit;
}

void
SendBuffer_free(SendBuffer* unit)
{
	StringBuffer_free(unit->buffer);
	cx_free(unit);
}
