#include "connection_worker.h"

void
ConnectionWorker_init(ConnectionWorker* worker)
{
	Worker_init((Worker*)worker);
	worker->worker.f_handler = ConnectionWorker_run;
}

ConnectionWorker*
ConnectionWorker_new(F_CreateConnection* f_connection_create, ConnectionCallbacks* callbacks)
{
	ConnectionWorker* worker = cx_alloc(sizeof(ConnectionWorker));

	ConnectionWorker_init(worker);
	worker->f_connection_create = f_connection_create;
	worker->callbacks = callbacks;
	return worker;
}

void
ConnectionWorker_run(Worker* worker)
{
	XDBG("Running connection worker");
	ConnectionWorker* connection_worker = (ConnectionWorker*)worker;

	connection_worker->server_fd = worker->server->socket->fd;

	ev_io_init(&connection_worker->connection_watcher,
		   connection_watcher, connection_worker->server_fd, EV_READ);
	ev_io_start(worker->loop, &connection_worker->connection_watcher);
	ev_run(worker->loop, 0);
	ev_io_stop(worker->loop, &connection_worker->connection_watcher);
}

/* creates new incomming connections */
void
connection_watcher(ev_loop* loop, ev_io* w, int revents)
{
	UNUSED(loop);
	UNUSED(revents);

	ConnectionWorker* connection_worker = container_of(w, ConnectionWorker, connection_watcher);
	Worker* worker = (Worker*)connection_worker;

	// TODO create a server socket
	int client_fd = accept(connection_worker->server_fd, NULL, NULL);

	if (client_fd == -1)
	{
		XFERRNO("worker[%lu] failed to accept connection", worker->id);
	}
	else
	{
		XFDBG("Worker[%lu] - accepted connection on fd:%d", worker->id, client_fd);
		Connection* connection = connection_worker->f_connection_create(connection_worker->callbacks);
		connection->fd = client_fd;
		connection->loop = worker->loop;
		connection->f_send_data_handler = Connection_write_simple;
		connection->send = Connection_send;
		ConnectionWorker_start_connection(worker->loop, connection);
	}
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

/* starts a new connection*/
void
ConnectionWorker_start_connection(ev_loop* loop, Connection* conn)
{
	CXDBG(conn, "start");

	unblock(conn->fd);
	ev_io_init(&conn->receive_data_watcher, receive_data_callback, conn->fd, EV_READ);
	ev_io_init(&conn->send_data_watcher, send_data_callback, conn->fd, EV_WRITE);
	ev_io_start(loop, &conn->receive_data_watcher);
//	ev_io_start(loop, &conn->send_data_watcher);

	Connection_callback(conn, on_start);
}

void
Connection_close_read(ev_loop* loop, Connection* conn)
{
	if (shutdown(conn->fd, SHUT_RD) == -1)
	{
		// remote end has already closed writing
		// ignore this only if EOF has been received previously ?
//		if (errno != ENOTCONN)
		CXERRNO(conn, "failed to shutdown reading end of connection");
	}

	ev_io_stop(loop, &conn->receive_data_watcher);
}

void
Connection_close_write(ev_loop* loop, Connection* conn)
{
	if (shutdown(conn->fd, SHUT_WR) == -1)
		CXERRNO(conn, "failed to shutdown writing end of connection");

	ev_io_stop(loop, &conn->send_data_watcher);
}

void
Connection_close(ev_loop* loop, Connection* conn)
{
	CXDBG(conn, "close");
	Connection_callback(conn, on_close);

	Connection_close_read(loop, conn);
	Connection_close_write(loop, conn);
	close(conn->fd);
	Connection_free(conn);
}

void
Connection_send(Connection* conn, Response* response)
{
	CXFDBG(conn, "send response %p", (void*)response);
	int res = Queue_add(conn->response_queue, response);
	assert(res == 0);
	ev_io_start(conn->loop, &conn->send_data_watcher);
}

void
Connection_write_simple(Connection* conn)
{
	Response* response = (Response*)List_first((List*)conn->response_queue);

	if (!response)
	{
		CXDBG(conn, "no response available for sending");
		ev_io_stop(conn->loop, &conn->send_data_watcher);
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

				// TODO replace with Connection_set_error(con, READ, ERRNO)
				conn->error = CONNECTION_ERROR_ERRNO;
				conn->error_errno = errno;
				CXERRNO(conn, "Failed to write response_data");
				// TODO add macro Connection_callback(on_error, conn);
				Connection_callback(conn, on_error);
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
		response = (Response*)Queue_pop(conn->response_queue); /* remove from list */
		Response_free(response);
	}
}
