#ifndef _CONNECTION_WORKER_H
#define _CONNECTION_WORKER_H

#include "worker.h"
#include "connection.h"

#define DATA_RECEIVE_MAX 1024

typedef struct cx_connection_worker_t ConnectionWorker;
typedef Connection* F_CreateConnection (ConnectionWorker* worker);

void
connection_watcher(ev_loop* loop, ev_io* w, int revents);

struct cx_connection_worker_t
{
	Worker worker;
	int server_fd;
	ev_io connection_watcher;
	F_CreateConnection* f_create_connection;
	ConnectionCallbacks* connection_callbacks;
};

ConnectionWorker*
ConnectionWorker_new(ConnectionCallbacks* connection_callbacks);

void
ConnectionWorker_init(ConnectionWorker* worker);

void
ConnectionWorker_run(Worker* worker);

#endif
