#include "libcx-base/test.h"
#include "libcx-base/base.h"
#include "server.h"

static void
unix_connection_watcher(ev_loop *loop, ev_io *w, int revents)
{
	Worker *worker = container_of(&loop, Worker, loop);
	UnixWorker *unix_worker = (UnixWorker*) worker;

	XFLOG("Worker[%lu] fd:%d - new connection (revents:%d)\n",
	      worker->id, w->fd, revents);

	int client_fd = accept(unix_worker->server_fd, NULL, NULL);
	if (client_fd == -1)
	{
		XFLOG("Worker[%lu] fd:%d - failed to accept\n", worker->id, w->fd);
		return;
	}
	else
	{
		enable_so_opt(client_fd, SO_NOSIGPIPE); /* do not send SIGIPIPE on EPIPE */
		XFLOG("Worker[%lu] fd:%d - accepted connection\n", worker->id, w->fd);

		Connection *connection = Connection_new(loop, client_fd, 128);
//		Task *task = Task_new(); ?
//		connection needs a proper connection event handler
//		connection events
		connection->f_event_handler(connection, CONNECTION_EVENT_NEW);
	}
}

/* control worker startup/shutdown */
static void
unix_worker_event_handler(Worker *worker, WorkerEvent event, void *data)
{
	UnixWorker *unix_worker = (UnixWorker *worker);
	switch(event)
	{
	case WORKER_EVENT_START:
		unix_worker->connection_watcher = malloc(sizeof(ev_io));
		unix_worker->requests = List_new();
		/* start connection watcher */
		break;
	case WORKER_EVENT_STOP:
		/* start shutdown watcher */
		ev_io_stop(unix_worker->connection_watcher);
		// worker should exit the event loop when all connections are handled
		pthread_cancel(worker->thread);
		List_free(unix_worker->requests);
		free(unix_worker->connection_watcher);
		break;
	}
}

/* builds the request, calls request handler */
static void
unix_connection_handler(Worker *worker, Connection *connection, ConnectionEvent event, void *userdata)
{
	UnixWorker *unix_worker = (UnixWorker *worker);
	switch(event)
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
		Request *request = connection->request;
		Message *message = request->message;
		Message_parse(message);
		break;
	case CONNECTION_EVENT_CLOSE_READ:
		// check if request is finished
		// push request to (verification) worker queue
//		List_push(unix_worker->requests, (void *) request);
		XDBG("close read");
		break;
	case CONNECTION_EVENT_NEW:
		// create new connection
		Request *request = Request_new();
		request->message = Message_new(2048);
		connection->request = request;
		break;
	}
}

// queue request | process request

/* worker calls the request handler for queued requests */
static void
request_handler(Request *request)
{
	// cast request to extended request

	// process the request

	// start request specific io watchers ?
}


int
main(int argc, char** argv)

{
	Server *server = Server_new("/tmp/echo.sock");
	server->worker_count = 4;
	Server_start(server);

	/* handles a connection, creates the request */
	server->f_connection_handler = unix_connection_handler;
	server->f_worker_handler = unix_worker_event_handler;
	server->f_request_handler = request_handler;

	server_stop(server);
}
