#include "unix_server.h"
#include "unix_worker.h" /* FIXME circular inclusion */

void
UnixWorker_init(UnixWorker *worker)
{
	Worker_init((Worker*)worker);
	worker->worker.f_handler = UnixWorker_run;
}

UnixWorker*
UnixWorker_new()
{
	UnixWorker *worker = malloc(sizeof(UnixWorker));

	UnixWorker_init(worker);
	return worker;
}

void
UnixWorker_run(Worker *worker)
{
	XDBG("Running unix worker");
	UnixWorker *unix_worker = (UnixWorker*)worker;
	UnixServer *server = (UnixServer*)worker->server;

	unix_worker->server_fd = server->fd;

	ev_io_init(&unix_worker->connection_watcher, unix_connection_watcher, server->fd, EV_READ);
	ev_io_start(worker->loop, &unix_worker->connection_watcher);
	ev_run(worker->loop, 0);
	ev_io_stop(worker->loop, &unix_worker->connection_watcher);
}

void
unix_connection_watcher(ev_loop *loop, ev_io *w, int revents)
{
	UnixWorker *unix_worker = container_of(w, UnixWorker, connection_watcher);
	Worker *worker = (Worker*)unix_worker;

	int client_fd = accept(unix_worker->server_fd, NULL, NULL);

	if (client_fd == -1)
		XFLOG("Worker[%lu] - failed to accept", worker->id);
	else
	{

#if (!defined (__linux__) && defined(__unix__)) || (defined(__APPLE__) && defined(__MACH__))
		/* do not send SIGIPIPE on EPIPE */
		enable_so_opt(client_fd, SO_NOSIGPIPE);
#endif

		XFLOG("Worker[%lu] - accepted connection on fd:%d", worker->id, client_fd);
		Connection *connection = unix_worker->f_create_connection();
		connection->worker = worker;
		connection->fd = client_fd;
		Connection_start(connection);
	}
}
