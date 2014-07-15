#include "connection_worker.h"

#define WORKERDATA(conn) \
	((ConnectionState*)conn->state)

typedef struct cx_connection_worker_data_t
{
	int fd;
	ConnectionWorker* worker;
	Connection* connection;

	ev_io receive_data_watcher;
	ev_io send_data_watcher;
	ev_async notify_send_data_watcher;
	ev_prepare close_connection_watcher;
	ev_timer timer_watcher;

	void* userdata;

	Response* response; /* response currently send progress */
} ConnectionState;

static void
connection_send_cb(ConnectionState* state);

static void
connection_close(Connection* conn);

static void
connection_close_receive(Connection* conn);

static void
connection_close_send(Connection* conn);

static void
connection_send(Connection* conn, Response* response, F_ResponseFinished* f_on_finished);

static void*
get_serverdata(Connection* conn)
{
	ConnectionState* state = WORKERDATA(conn);

	return ((Worker*)state->worker)->server->data;
}

static int
get_id(Connection* conn)
{
	return WORKERDATA(conn)->fd;
}

static inline void
enable_receive(Connection* conn)
{
	CXDBG(conn, "receive start");
	ConnectionState* state = WORKERDATA(conn);

	ev_io_start(state->worker->loop, &state->receive_data_watcher);
}

static inline void
disable_receive(Connection* conn)
{
	CXDBG(conn, "receive stop");
	ConnectionState* state = WORKERDATA(conn);

	ev_io_stop(state->worker->loop, &state->receive_data_watcher);
}

static inline void
enable_send(Connection* conn)
{
	CXDBG(conn, "enable send");
	ConnectionState* state = WORKERDATA(conn);

	ev_io_start(state->worker->loop, &state->send_data_watcher);
}

static inline void
disable_send(Connection* conn)
{
	CXDBG(conn, "disable send");
	ConnectionState* state = WORKERDATA(conn);

	ev_io_stop(state->worker->loop, &state->send_data_watcher);
}

static inline void
start_timer(Connection* conn, unsigned millis)
{
	CXFDBG(conn, "start timer (timeout %u millis)", millis);
	ConnectionState* state = WORKERDATA(conn);

	if (state->connection->callbacks->on_timeout)
	{
		state->timer_watcher.repeat = millis / 1000.0;
		ev_timer_again(state->worker->loop, &state->timer_watcher);
	}
	else
	{
		XERR("No timer callback set. Refusing to enable timer.");
	}
}

static inline void
stop_timer(Connection* conn)
{
	CXDBG(conn, "stop timer");
	ConnectionState* state = WORKERDATA(conn);

	ev_timer_stop(state->worker->loop, &state->timer_watcher);
}

static void
timer_watcher_callback(ev_loop* loop, ev_timer* w, int revents)
{
	UNUSED(loop);
	UNUSED(revents);

	ConnectionState* state = container_of(w, ConnectionState, timer_watcher);

	UNUSED(state);

	state->connection->callbacks->on_timeout(state->connection);
}

static void
connection_watcher(ev_loop* loop, ev_io* w, int revents);

static void
receive_data_callback(ev_loop* loop, ev_io* w, int revents)
{
	UNUSED(loop);

	ConnectionState* state = container_of(w, ConnectionState, receive_data_watcher);

	assert(revents & EV_READ);  /* application bug */

	if (state->connection->f_receive)
		state->connection->f_receive(state->connection, state->fd);
}

static void
send_data_callback(ev_loop* loop, ev_io* w, int revents)
{
	UNUSED(loop);

	ConnectionState* state = container_of(w, ConnectionState, send_data_watcher);

	assert(revents & EV_WRITE);  /* application bug */

	connection_send_cb(state);
}

static void
enable_send_data_watcher(ev_loop* loop, ev_async* w, int revents)
{
	UNUSED(loop);
	UNUSED(revents);

	ConnectionState* state = container_of(w, ConnectionState, notify_send_data_watcher);
	enable_send(state->connection);
}

/* FIXME free !!! */
static ConnectionState*
ConnectionState_new(ConnectionWorker* worker, Connection* conn, int fd)
{
	ConnectionState* state = cx_alloc(sizeof(ConnectionState));

	state->worker = worker;
	state->connection = conn;
	state->fd = fd;
	ev_io_init(&state->receive_data_watcher, &receive_data_callback, fd, EV_READ);
	ev_io_init(&state->send_data_watcher, &send_data_callback, fd, EV_WRITE);
	ev_async_init(&state->notify_send_data_watcher, &enable_send_data_watcher);
	ev_init(&state->timer_watcher, &timer_watcher_callback);
	unblock(fd);
	return state;
}

static void
ConnectionState_free(ConnectionState* state)
{
	if (state->response)
		Response_free(state->response);
	cx_free(state);
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
static void
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
		ConnectionState* state = ConnectionState_new(connection_worker, conn, client_fd);
		conn->f_free_state = (F_FreeState*)ConnectionState_free;

		conn->state = state;

		/* Register API methods which use on the worker data */
		conn->f_send = connection_send;
		conn->f_close = connection_close;
		conn->f_receive_close = connection_close_receive;
		conn->f_send_close = connection_close_send;
		conn->f_get_serverdata = get_serverdata;
		conn->f_get_id = get_id;

		conn->f_receive_enable = enable_receive;
		conn->f_receive_disable = disable_receive;
		conn->f_send_enable = enable_send;
		conn->f_send_disable = disable_send;

		conn->f_timer_start = start_timer;
		conn->f_timer_stop = stop_timer;

		Connection_callback(conn, on_connect);

		enable_receive(conn);
		ev_async_start(loop, &state->notify_send_data_watcher);
	}
}

static void
connection_close_receive(Connection* conn)
{
	ConnectionState* state = WORKERDATA(conn);

	disable_receive(conn);

	if (shutdown(state->fd, SHUT_RD) == -1)
	{
		// remote end has already closed writing
		// ignore this only if EOF has been received previously ?
//		if (errno != ENOTCONN)
//		CXERRNO(conn, "failed to shutdown reading end of connection");
	}
}

static void
connection_close_send(Connection* conn)
{
	ConnectionState* state = WORKERDATA(conn);

	disable_send(conn);

	if (shutdown(state->fd, SHUT_WR) == -1)
		CXERRNO(conn, "failed to shutdown writing end of connection");
}

static void
close_connection_cb(ev_loop* loop, ev_prepare* w, int revents)
{
	UNUSED(revents);
	ConnectionState* state = container_of(w, ConnectionState, close_connection_watcher);
	ev_prepare_stop(loop, w);

	XFDBG("Destroying connection %d", state->fd);
	close(state->fd);
	Connection_free(state->connection);
}

static void
connection_close(Connection* conn)
{
	CXDBG(conn, "close");

	ConnectionState* state = WORKERDATA(conn);
	Connection_callback(conn, on_close);

	ev_async_stop(state->worker->loop, &state->notify_send_data_watcher);
	stop_timer(conn);
	disable_receive(conn);
	disable_send(conn);

	/* schedule connection close to beginning of next loop iteration */
	ev_prepare_init(&state->close_connection_watcher, &close_connection_cb);
	ev_prepare_start(state->worker->loop, &state->close_connection_watcher);
}

static void
connection_send(Connection* conn, Response* response, F_ResponseFinished* f_on_finished)
{
	CXFDBG(conn, "send response %p", (void*)response);

	response->on_finished = f_on_finished;

	ConnectionState* state = WORKERDATA(conn);
	int res = Queue_add(conn->response_queue, response);
	assert(res == 0);
	ev_async_send(state->worker->loop, &state->notify_send_data_watcher);
}

static void
write_response(Connection* conn, Response* response, int fd)
{
	if (response->f_data_available(response))
	{
		const char* response_data = NULL;
		size_t response_data_len = response->f_data_get(response, &response_data);

		if (response_data_len > 0)
		{
			ssize_t nwritten = write(fd, response_data, response_data_len);
			if (nwritten == -1)
			{
				CXERRNO(conn, "Failed to write response_data");
				/* we should never receive EAGAIN here (only for blocking IO) */
				assert(errno != EAGAIN);
				Connection_errno_write(conn);
			}
			else
			{
				CXFDBG(conn, "%zd bytes send", nwritten);
				response->on_data_transmitted(response, (size_t)nwritten);
			}
		}
	}
}

static void
connection_send_cb(ConnectionState* state)
{
	Connection* conn = state->connection;

	Response* response = NULL;

	if (!state->response)
		state->response = (Response*)Queue_get(conn->response_queue);

	response = state->response;

	if (response)
	{
		if (response->f_data_available(response))
		{
			CXFDBG(conn, "writing response [%p]", (void*)response);
			write_response(conn, response, state->fd);
		}

		if (!response->f_data_available(response))
		{
			if (response->on_finished)
				response->on_finished(response, (void*)conn);

			Response_free(response);
			state->response = NULL;
		}
	}
	else
	{
		CXDBG(conn, "no more queued responses");
		disable_send(conn);
	}
}
