#include "connection_worker.h"

typedef struct cx_connection_worker_data_t
{
	int fd;
	ConnectionWorker* worker;
	Connection* connection;

	ev_io receive_data_watcher;
	ev_io send_data_watcher;
} ConnectionWorkerData;

static void
connection_close(Connection* conn);

static void
connection_close_read(Connection* conn);

static void
connection_close_write(Connection* conn);

static void
connection_queue_response(Connection* conn, Response* response);

static void
connection_write_cb(ConnectionWorkerData* conn);

static void
receive_data_callback(ev_loop* loop, ev_io* w, int revents)
{
	UNUSED(loop);

	ConnectionWorkerData* data = container_of(w, ConnectionWorkerData, receive_data_watcher);

	assert(revents & EV_READ);  /* application bug */

	if (data->connection->f_receive)
		data->connection->f_receive(data->connection, data->fd);
}

static void
send_data_callback(ev_loop* loop, ev_io* w, int revents)
{
	UNUSED(loop);

	ConnectionWorkerData* data = container_of(w, ConnectionWorkerData, send_data_watcher);

	assert(revents & EV_WRITE);  /* application bug */

	connection_write_cb(data);
}

/* FIXME free !!! */
static ConnectionWorkerData*
ConnectionWorkerData_new(ConnectionWorker* worker, Connection* conn, int fd)
{
	ConnectionWorkerData* data = cx_alloc(sizeof(ConnectionWorkerData));

	data->worker = worker;
	data->connection = conn;
	data->fd = fd;
	ev_io_init(&data->receive_data_watcher, receive_data_callback, fd, EV_READ);
	ev_io_init(&data->send_data_watcher, send_data_callback, fd, EV_WRITE);
	unblock(fd);
	ev_io_start(worker->loop, &data->receive_data_watcher);
	return data;
}

static void
ConnectionWorkerData_free(ConnectionWorkerData* data)
{
	cx_free(data);
}

ConnectionWorker*
ConnectionWorker_new(F_CreateConnection* f_connection_create, ConnectionCallbacks* callbacks)
{
	ConnectionWorker* worker = cx_alloc(sizeof(ConnectionWorker));

	Worker_init((Worker*)worker);
	worker->worker.f_handler = ConnectionWorker_run;

	worker->loop = ev_loop_new(EVBACKEND);

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
	ev_io_start(connection_worker->loop, &connection_worker->connection_watcher);
	ev_run(connection_worker->loop, 0);        /* blocks until worker is stopped */
	ev_io_stop(connection_worker->loop, &connection_worker->connection_watcher);
}

/* creates new incomming connections */
void
connection_watcher(ev_loop* loop, ev_io* w, int revents)
{
	UNUSED(loop);
	UNUSED(revents);

	ConnectionWorker* connection_worker = container_of(w, ConnectionWorker, connection_watcher);
	Worker* worker = (Worker*)connection_worker;

	int client_fd = accept(connection_worker->server_fd, NULL, NULL);
	if (client_fd == -1)
	{
		XFERRNO("worker[%lu] failed to accept connection", worker->id);
	}
	else
	{
		XFDBG("Worker[%lu] - accepted connection on fd:%d", worker->id, client_fd);
		Connection* conn = connection_worker->f_connection_create(connection_worker->callbacks);

		conn->worker_data = ConnectionWorkerData_new(connection_worker, conn, client_fd);

		/* Register API methods which use on the worker data */
		conn->f_send = connection_queue_response;
		conn->f_close = connection_close;
		conn->f_close_read = connection_close_read;
		conn->f_close_write = connection_close_write;

		Connection_callback(conn, on_start);
	}
}

static void
connection_close_read(Connection* conn)
{
	ConnectionWorkerData* conn_data = (ConnectionWorkerData*)conn->worker_data;

	if (shutdown(conn_data->fd, SHUT_RD) == -1)
	{
		// remote end has already closed writing
		// ignore this only if EOF has been received previously ?
//		if (errno != ENOTCONN)
//		CXERRNO(conn, "failed to shutdown reading end of connection");
	}

	ev_io_stop(conn_data->worker->loop, &conn_data->receive_data_watcher);
}

static void
connection_close_write(Connection* conn)
{
	ConnectionWorkerData* conn_data = (ConnectionWorkerData*)conn->worker_data;

	if (shutdown(conn_data->fd, SHUT_WR) == -1)
		CXERRNO(conn_data, "failed to shutdown writing end of connection");

	ev_io_stop(conn_data->worker->loop, &conn_data->send_data_watcher);
}

static void
connection_close(Connection* conn)
{
	ConnectionWorkerData* conn_data = (ConnectionWorkerData*)conn->worker_data;

	CXDBG(conn_data, "close");
	Connection_callback(conn, on_close);

	connection_close_read(conn);
	connection_close_write(conn);
	close(conn_data->fd);
	Connection_free(conn);
	ConnectionWorkerData_free(conn_data);
}

static void
connection_queue_response(Connection* conn, Response* response)
{
	ConnectionWorkerData* conn_data = (ConnectionWorkerData*)conn->worker_data;

	CXFDBG(conn_data, "send response %p", (void*)response);
	int res = Queue_add(conn->response_queue, response);
	assert(res == 0);
	ev_io_start(conn_data->worker->loop, &conn_data->send_data_watcher);
}

static void
handle_send_error(Connection* conn)
{
	/* we should never receive EAGAIN here */
	assert(errno != EAGAIN);
	// TODO replace with Connection_set_error(con, READ, ERRNO)
	conn->error = CONNECTION_ERROR_ERRNO;
	conn->error_errno = errno;
	Connection_callback(conn, on_error);
}

static void
connection_write_cb(ConnectionWorkerData* conn_data)
{
	Connection* conn = conn_data->connection;
	Response* response = (Response*)List_first((List*)conn->response_queue);

	if (!response)
	{
		CXDBG(conn_data, "no response available for sending");
		ev_io_stop(conn_data->worker->loop, &conn_data->send_data_watcher);
		return;
	}

	CXFDBG(conn_data, "writing response [%p]", (void*)response);

	if (response->data_available(response))
	{
		const char* response_data = NULL;
		size_t response_data_len = response->data_get(response, &response_data);

		if (response_data_len > 0)
		{
			ssize_t nwritten = write(conn_data->fd, response_data, response_data_len);
			if (nwritten == -1)
			{
				CXERRNO(conn_data, "Failed to write response_data");
				handle_send_error(conn);
			}
			else
			{
				CXFDBG(conn_data, "%zd bytes send", nwritten);
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
