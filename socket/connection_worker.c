#include "connection_worker.h"

void
ConnectionWorker_init(ConnectionWorker* worker)
{
	Worker_init((Worker*)worker);
	worker->worker.f_handler = ConnectionWorker_run;
}

ConnectionWorker*
ConnectionWorker_new(ConnectionCallbacks* connection_callbacks)
{
	ConnectionWorker* worker = cx_alloc(sizeof(ConnectionWorker));

	ConnectionWorker_init(worker);
	worker->connection_callbacks = connection_callbacks;
	return worker;
}

void
ConnectionWorker_run(Worker* worker)
{
	XDBG("Running unix worker");
	ConnectionWorker* connection_worker = (ConnectionWorker*)worker;

	connection_worker->server_fd = worker->server->socket->fd;

	ev_io_init(&connection_worker->connection_watcher,
		   connection_watcher, connection_worker->server_fd, EV_READ);
	ev_io_start(worker->loop, &connection_worker->connection_watcher);
	ev_run(worker->loop, 0);
	ev_io_stop(worker->loop, &connection_worker->connection_watcher);
}

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
		Connection* connection = connection_worker->f_create_connection(connection_worker);
		connection->worker = worker;
		connection->fd = client_fd;
		Connection_start(connection);
	}
}
