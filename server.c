#include "server.h"

static void
shutdown_watcher(ev_loop *loop, ev_timer *w, int revents);

static void
sigint_watcher(ev_loop *loop, ev_signal *w, int revents);

static void
free_worker(void *data)
{
	Worker *worker = (Worker*)data;

	Worker_stop(worker); // FIXME must signal worker to close all connections
	Worker_free(worker);
}

// TODO default to processor count (varies from machine to machine)
#define PROCESSOR_COUNT 4

void
Server_new(Server *server)
{
	server->worker_count = PROCESSOR_COUNT;
	server->backlog = 0;                         // TODO set reasonable default (currently unused)
	server->loop = EV_DEFAULT;
	server->workers = List_new();
	server->workers->f_node_data_free = free_worker;
}

int
Server_start(Server *server)
{
	server->f_server_handler(server, SERVER_START, NULL);

	// start workers
	int i;
	for (i = 0; i < server->worker_count; i++)
	{
		// create a custom worker instance
		Worker *worker = server->f_worker_handler(NULL, WORKER_EVENT_NEW);

		worker->id = i;
		worker->f_handler = server->f_worker_handler;
		worker->f_connection_handler = server->f_connection_handler;
		worker->f_connection_data_handler = server->f_connection_data_handler;
		List_push(server->workers, worker);

		// callback allows custom initialization of worker
		server->f_server_handler(server, SERVER_START_WORKER, worker);

		Worker_start(worker);
	}

	/* shutdown server on SIGINT */
	signal(SIGINT, SIG_IGN); /* SIGINT is handled by a callback */
	ev_signal_init(&server->sigint_watcher, sigint_watcher, SIGINT);
	ev_signal_start(server->loop, &server->sigint_watcher);

	ev_run(server->loop, 0);

	// TODO error handling
	return 0;
}

void
Server_free(Server *server)
{
	List_free(server->workers);
	free(server);
}

void
Server_stop(Server *server)
{
	Server_free(server);
}

void
enable_so_opt(int fd, int option)
{
	int enable = 1;

	setsockopt(fd, SOL_SOCKET, option, (void*)&enable, sizeof(enable));
}

static void
shutdown_watcher(ev_loop *loop, ev_timer *w, int revents)
{
	Server *server = container_of(w, Server, shutdown_watcher);

	XDBG("Waiting for workers to shut down");
	// TODO wait here for workers to shutdown

	ev_timer_stop(loop, w);
	ev_break(loop, EVBREAK_ALL);
}

/* handle SIGINT callback (starts the shutdown timer) */
static void
sigint_watcher(ev_loop *loop, ev_signal *w, int revents)
{
	Server *server = container_of(w, Server, sigint_watcher);

	XDBG("Received SIGINT. Starting shutdown timer.");
	// TODO send async to workers to shutdown

	server->f_server_handler(server, SERVER_STOP, NULL);

	ev_timer_init(&server->shutdown_watcher, shutdown_watcher, 0., 1.);
	ev_timer_again(loop, &server->shutdown_watcher);
}
