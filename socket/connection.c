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
	Response_free((Response*)data);
}

void
Connection_init(Connection* connection, Worker* worker, int fd)
{
	connection->worker = worker;
	connection->fd = fd;
	connection->response_list = List_new();
	connection->response_list->f_node_data_free = send_buffer_node_free;
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
	List_free(conn->response_list);
	cx_free(conn);
}

void
Connection_start(Connection* conn)
{
	CXDBG(conn, "start");

	unblock(conn->fd);
	ev_io_init(&conn->receive_data_watcher, receive_data_callback, conn->fd, EV_READ);
	ev_io_init(&conn->send_data_watcher, send_data_callback, conn->fd, EV_WRITE);
	ev_io_start(conn->worker->loop, &conn->receive_data_watcher);

	if (conn->handler.on_start)
		conn->handler.on_start(conn);
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
Connection_close(Connection* conn)
{
	CXDBG(conn, "close");

	if (conn->handler.on_close)
		conn->handler.on_close(conn);

	Connection_close_read(conn);
	Connection_close_write(conn);
	close(conn->fd);
	Connection_free(conn);
}

static void
connection_write(Connection* conn)
{
	Response* response = (Response*)List_get(conn->response_list, 0);

	if (!response)
	{
		CXDBG(conn, "no response available for sending");
		Connection_stop_write(conn);
		return;
	}

	CXFDBG(conn, "writing response [%p]", (void*)response);

	if (response->data_available(response))
	{
		const char* response_data = NULL;
		size_t response_data_len = response->data_get(response, &response_data);

		if (response_data_len > 0)
		{
			ssize_t nwritten = write(conn->fd, response_data, response_data_len);
			if (nwritten == -1)
			{
				/* we should never receive EAGAIN here */
				assert(errno != EAGAIN);
				CXERRNO(conn, "Failed to write response_data");
				if (conn->f_on_write_error)
					conn->f_on_write_error(conn);
			}
			else
			{
				CXFDBG(conn, "%zd bytes send", nwritten);
				response->on_data_transmitted(response, (size_t)nwritten);
			}
		}
	}

	if (!response->data_available(response))
	{
		response = (Response*)List_shift(conn->response_list); /* remove from list */
		if (response->on_finished)
			response->on_finished(conn, response);
		Response_free(response);
	}
}

void
Connection_send(Connection* conn, Response* response)
{
	List_push(conn->response_list, response);
	Connection_start_write(conn);
}
