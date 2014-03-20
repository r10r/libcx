#include "libcx-base/test.h"
#include "libcx-base/base.h"
#include "server.h"


static void
unix_connection_watcher(ev_loop *loop, ev_io *w, int revents)
{
	UnixWorker *unix_worker = container_of(w, UnixWorker, connection_watcher);
	Worker *worker = (Worker*)unix_worker;

	int client_fd = accept(unix_worker->server_fd, NULL, NULL);

	if (client_fd == -1)
		XFLOG("Worker[%lu] - failed to accept\n", worker->id);
	else
	{
		/* do not send SIGIPIPE on EPIPE */
		enable_so_opt(client_fd, SO_NOSIGPIPE);
		XFLOG("Worker[%lu] - accepted connection on fd:%d\n", worker->id, client_fd);

		Connection *connection = Connection_new(loop, client_fd, 128);
		connection->f_handler = worker->f_connection_handler;
		connection->f_handler(connection, CONNECTION_EVENT_ACCEPTED);
	}
}

static void
unix_worker_handler(Worker *worker, WorkerEvent event)
{
	UnixWorker *unix_worker = (UnixWorker*)worker;

	switch (event)
	{
	case WORKER_EVENT_START:
		XFDBG("worker event start worker:%lu, loop:%p\n", worker->id, worker->loop);
		unix_worker->requests = List_new();

		/* start connection watcher */
		ev_io_init(&unix_worker->connection_watcher, unix_connection_watcher, unix_worker->server_fd, EV_READ);
		ev_io_start(worker->loop, &unix_worker->connection_watcher);

		XDBG("worker event start finished");
		break;
	case WORKER_EVENT_STOP:
		XDBG("worker event stop");
		/* start shutdown watcher */
		ev_io_stop(worker->loop, &unix_worker->connection_watcher);
		// worker should exit the event loop when all connections are handled
		pthread_cancel(*worker->thread);
		List_free(unix_worker->requests);
		break;
	case WORKER_EVENT_RELEASE:
		// TODO release worker
		break;
	}
}

static void
unix_connection_handler(Connection *connection, ConnectionEvent event)
{
	switch (event)
	{
	case CONNECTION_EVENT_RECEIVE_TIMEOUT:
		XDBG("connection timeout");
		break;
	case CONNECTION_EVENT_ERRNO:
		XDBG("connection error");
		break;
	case CONNECTION_EVENT_ERROR_WRITE:
		XDBG("connection error write");
		break;
	case CONNECTION_EVENT_RECEIVE_DATA:
	{
		/* data has been written to the message buffer */
		Message_parse(connection->request->message);
		break;
	}
	case CONNECTION_EVENT_CLOSE_READ:
		XDBG("close read");
		Message_parse_finish(connection->request->message);
		ev_io_stop(connection->loop, &connection->receive_data_watcher);
		break;
	case CONNECTION_EVENT_ACCEPTED:
		XDBG("connection accepted");
		Request *request = Request_new(666);
		request->message = Message_new(2048);
		connection->request = request;
		Connection_start(connection);
		break;
	case CONNECTION_EVENT_NEW_MESSAGE:
	{
		XDBG("new message");
		break;
	}
	case CONNECTION_EVENT_END:
		XDBG("connection end");
		break;
	}
}

/* worker calls the request handler for queued requests */
static void
unix_request_handler(Request *request)
{
	// cast request to extended request

	// process the request

	// start request specific io watchers ?
}

static void
timer_cb(ev_loop *loop, ev_timer *timer, int revents)
{
	printf("Hello from server\n");
}

static void
unix_server_handler(Server *server, ServerEvent event, void *data)
{
	UnixServer *unix_server = (UnixServer*)server;

	switch (event)
	{
	case SERVER_START:
	{
		// connect server to socket
		unix_server->socket_path = "/tmp/echo.sock";
		unix_server->fd = unix_socket_connect(unix_server->socket_path);

		// TODO start worker supervisor
		ev_timer *timer = malloc(sizeof(ev_timer));
		ev_timer_init(timer, timer_cb, 0., 15.);
		ev_timer_again(EV_DEFAULT, timer);
		break;
	}
	case SERVER_STOP:
		break;
	case WORKER_START:
	{
		// pass server socket from server to worker
		UnixWorker *worker = (UnixWorker*)data;
		worker->server_fd = unix_server->fd;
	}
	break;
	}
}

int
main(int argc, char** argv)

{
	Server *server = Server_new();

	server->worker_count = 4;

	/* handles a connection, creates the request */
	server->f_connection_handler = unix_connection_handler;
	server->f_worker_handler = unix_worker_handler;
	server->f_request_handler = unix_request_handler;
	server->f_server_handler = unix_server_handler;

	int ret = Server_start(server);
	XASSERT(ret == 0, "server should have been started");

	Server_stop(server);
}
