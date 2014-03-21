#include "server.h"

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
		server->f_server_handler(server, WORKER_START, worker);

		Worker_start(worker);
	}

	// TODO start SIGINT handler to handle server shutdown
	ev_run(server->loop, 0);

	server->f_server_handler(server, SERVER_STOP, NULL);
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
