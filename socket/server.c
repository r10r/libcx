#include "server.h"

static void
shutdown_watcher(ev_loop* loop, ev_timer* w, int revents);

static void
sigint_watcher(ev_loop* loop, ev_signal* w, int revents);

static void
free_worker(void* data)
{
	Worker* worker = (Worker*)data;

	Worker_stop(worker);
}

Server*
Server_new()
{
	Server* server = cx_alloc(sizeof(Server));

	Server_init(server);
	return server;
}

void
Server_init(Server* server)
{
	server->backlog = SOCK_BACKLOG;    // TODO set reasonable default (currently unused)
	server->loop = ev_default_loop(EVBACKEND);
	server->workers = List_new();
	server->workers->f_node_data_free = free_worker;
}

void
Server_free(Server* server)
{
	List_free(server->workers);
	cx_free(server);
}

int
Server_start(Server* server)
{
	server->f_server_handler(server, SERVER_START);

	// start workers
	unsigned int i;
	for (i = 0; i < (unsigned int)server->workers->length; i++)
	{
		Worker* worker = (Worker*)List_get(server->workers, i);
		worker->id = i;
		worker->server = server;
		Worker_start(worker);
	}

	/* shutdown server on SIGINT */
	signal(SIGINT, SIG_IGN); /* SIGINT is handled by a callback */
	ev_signal_init(&server->sigint_watcher, sigint_watcher, SIGINT);
	ev_signal_start(server->loop, &server->sigint_watcher);

	/* initialize shutdown timer */
	ev_init(&server->shutdown_watcher, shutdown_watcher);

	ev_run(server->loop, 0);

	return 0;
}

void
Server_shutdown(Server* server)
{
	// shutdown callback
	server->f_server_handler(server, SERVER_SHUTDOWN);

	// start shutdown watcher
	server->shutdown_watcher.repeat = 1.;
	ev_timer_again(server->loop, &server->shutdown_watcher);
}

static void
shutdown_watcher(ev_loop* loop, ev_timer* w, int revents)
{
	UNUSED(revents);

	Server* server = container_of(w, Server, shutdown_watcher);

	XDBG("Waiting for workers to shut down");
	// TODO wait here for workers to shutdown
	ev_timer_stop(loop, w);
	ev_break(loop, EVBREAK_ALL);
	Server_free(server);
}

/* handle SIGINT callback (starts the shutdown timer) */
static void
sigint_watcher(ev_loop* loop, ev_signal* w, int revents)
{
	UNUSED(loop);
	UNUSED(revents);

	Server* server = container_of(w, Server, sigint_watcher);

	XDBG("Received SIGINT.");
	Server_shutdown(server);
}
