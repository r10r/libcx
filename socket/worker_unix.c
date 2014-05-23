#include "server_unix.h"
#include "worker_unix.h"        /* FIXME circular inclusion */

void
UnixWorker_init(UnixWorker* worker)
{
	Worker_init((Worker*)worker);
	worker->worker.f_handler = UnixWorker_run;
}

UnixWorker*
UnixWorker_new()
{
	UnixWorker* worker = cx_alloc(sizeof(UnixWorker));

	UnixWorker_init(worker);
	return worker;
}

void
UnixWorker_run(Worker* worker)
{
	XDBG("Running unix worker");
	UnixWorker* unix_worker = (UnixWorker*)worker;

	unix_worker->server_fd = worker->server->socket->fd;

	ev_io_init(&unix_worker->connection_watcher,
		   unix_connection_watcher, unix_worker->server_fd, EV_READ);
	ev_io_start(worker->loop, &unix_worker->connection_watcher);
	ev_run(worker->loop, 0);
	ev_io_stop(worker->loop, &unix_worker->connection_watcher);
}

void
unix_connection_watcher(ev_loop* loop, ev_io* w, int revents)
{
	UNUSED(loop);
	UNUSED(revents);

	UnixWorker* unix_worker = container_of(w, UnixWorker, connection_watcher);
	Worker* worker = (Worker*)unix_worker;

	int client_fd = accept(unix_worker->server_fd, NULL, NULL);

	if (client_fd == -1)
		XFERRNO("worker[%lu] failed to accept connection", worker->id);
	else
	{
#ifdef _DARWIN_C_SOURCE
		/* do not send SIGIPIPE on EPIPE */
		enable_so_opt(client_fd, SO_NOSIGPIPE);
#endif

		XFDBG("Worker[%lu] - accepted connection on fd:%d", worker->id, client_fd);
		Connection* connection = unix_worker->f_create_connection();
		connection->worker = worker;
		connection->fd = client_fd;
		Connection_start(connection);
	}
}
