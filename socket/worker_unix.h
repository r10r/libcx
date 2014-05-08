#ifndef _CONNECTION_WORKER_H
#define _CONNECTION_WORKER_H

#include "worker.h"
#include "connection.h"

#define DATA_RECEIVE_MAX 1024

typedef Connection* F_CreateConnection ();

void
unix_connection_watcher(ev_loop* loop, ev_io* w, int revents);


typedef struct cx_unix_worker_t
{
	Worker worker;
	int server_fd;
	ev_io connection_watcher;
	F_CreateConnection* f_create_connection;
} UnixWorker;

UnixWorker*
UnixWorker_new(void);

void
UnixWorker_init(UnixWorker* worker);

void
UnixWorker_run(Worker* worker);

#endif
