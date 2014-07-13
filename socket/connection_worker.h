#ifndef _CONNECTION_WORKER_H
#define _CONNECTION_WORKER_H

#include <libcx/base/ev.h>

#include "worker.h"
#include "connection.h"

#define DATA_RECEIVE_MAX 1024

typedef struct cx_connection_worker_t ConnectionWorker;
typedef Connection* F_CreateConnection (ConnectionCallbacks* callbacks);
typedef Connection* F_ManageConnection (ConnectionWorker* worker, Connection* connection);

struct cx_connection_worker_t
{
	Worker worker;
	int server_fd;
	ev_loop* loop;
	ev_io connection_watcher;

	ConnectionCallbacks* callbacks;
	F_CreateConnection* f_connection_create;
};

ConnectionWorker*
ConnectionWorker_new(F_CreateConnection* f_connection_create, ConnectionCallbacks* connection_callbacks);

void
ConnectionWorker_run(Worker* worker);

#endif
