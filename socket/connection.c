#include "connection.h"

static void
connection_write(Connection* conn);

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

	connection->f_send_data_handler = connection_write;
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

static void
connection_write(Connection* conn)
{
	SendBuffer* send_buffer = (SendBuffer*)List_get(conn->send_buffers, 0);

	if (!send_buffer)
	{
		CXDBG(conn, "no more units available for sending");
		Connection_stop_write(conn);
		return;
	}

	CXFDBG(conn, "write data [%p]", (void*)send_buffer->buffer);
	size_t ntransmit = StringBuffer_used(send_buffer->buffer) - send_buffer->ntransmitted;
	if (ntransmit == 0)
	{
		CXDBG(conn, "no more data available for writing");
		List_shift(conn->send_buffers); /* remove from list */
		if (send_buffer->f_send_finished)
			send_buffer->f_send_finished(conn, send_buffer);
		SendBuffer_free(send_buffer);
	}
	else
	{
		char* start = StringBuffer_value(send_buffer->buffer) + send_buffer->ntransmitted;
		ssize_t nwritten = write(conn->fd, start, ntransmit);

		if (nwritten == -1)
		{
			/* we should never receive EAGAIN here */
			assert(errno != EAGAIN);
			CXERRNO(conn, "Failed to write data");
			if (conn->f_on_write_error)
				conn->f_on_write_error(conn);
		}
		else
		{
#ifdef _CX_DEBUG
			StringBuffer_print_bytes_hex(send_buffer->buffer, 16, "bytes send");
#endif
			CXFDBG(conn, "send %zu bytes (%zu remaining)", nwritten, ntransmit - (size_t)nwritten);
			send_buffer->ntransmitted += (size_t)nwritten;
		}
	}
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
