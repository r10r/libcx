#include "server.h"

static void
shutdown_watcher(ev_loop* loop, ev_timer* w, int revents);

static void
sigint_watcher(ev_loop* loop, ev_signal* w, int revents);

static void
free_worker(void* data)
{
	Worker* worker = (Worker*)data;

	Worker_free(worker);
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
	server->f_start(server);

	Server_callback(server, on_start);

	/* ignore signals before spawning workers */
	/* ignore SIGPIPE -  write will return SIGPIPE */
	signal(SIGPIPE, SIG_IGN);
	/* ignore SIGINT - shutdown handler is attached to SIGINT */
	signal(SIGINT, SIG_IGN);

	/* handle SIGINT in a callback */
	ev_signal* sigint_w = &server->sigint_watcher;
	ev_signal_init(sigint_w, sigint_watcher, SIGINT);
	ev_signal_start(server->loop, &server->sigint_watcher);

	/* initialize shutdown timer */
	ev_timer* shutdown_w = &server->shutdown_watcher;
	ev_init(shutdown_w, shutdown_watcher);

	/* start workers */
	unsigned int num_workers = (unsigned int)server->workers->length;

	unsigned int i;
	for (i = 0; i < num_workers; i++)
	{
		Worker* worker = (Worker*)List_get(server->workers, i);
		worker->id = i;
		worker->server = server;
		Worker_start(worker);
	}
	XFLOG("%d workers started.", num_workers);


	/* blocks until SIGINT handler fires and initiates the shutdown process */
	ev_run(server->loop, 0);
	return 0;
}

void
Server_stop(Server* server)
{
	// shutdown callback
	server->f_stop(server);

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

	Node* iter = server->workers->first;
	Node* elem = NULL;
	LIST_EACH(iter, elem)
	{
		Worker_stop((Worker*)elem->data);
	}

	Server_callback(server, on_stop);
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
	Server_stop(server);
}
