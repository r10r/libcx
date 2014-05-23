#ifndef _CX_WORKER_H
#define _CX_WORKER_H

#include <pthread.h>

#include "list/list.h"
#include "base/ev.h"

typedef struct cx_worker_t Worker;

typedef void F_WorkerHandler (Worker* worker);

struct cx_worker_t
{
	/* statistics ? */
	unsigned long id;               /* the worker id */
	pthread_t* thread;              /* the worker thread */
	ev_loop* loop;                  /* the workers event loop */
	Server* server;

	F_WorkerHandler* f_handler;
};

Worker*
Worker_new(void);

void
Worker_init(Worker* worker);

void
Worker_free(Worker* worker);

int
Worker_start(Worker* worker);

void
Worker_stop(Worker* worker);

#endif
