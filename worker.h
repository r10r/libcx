#ifndef _CX_WORKER_H
#define _CX_WORKER_H

#include <pthread.h>

#include "libcx-base/debug.h"
#include "libcx-list/list.h"
#include "libcx-base/ev.h"

typedef struct worker_t Worker;

typedef void F_WorkerHandler (Worker *worker);

struct worker_t
{
	/* statistics ? */
	unsigned long id;               /* the worker id */
	pthread_t *thread;              /* the worker thread */
	ev_loop *loop;                  /* the workers event loop */
	Server *server;

	F_WorkerHandler *f_handler;
};

Worker*
Worker_new(Worker *worker);

void
Worker_init(Worker *worker);

void
Worker_free(Worker *worker);

int
Worker_start(Worker *worker);

void
Worker_stop(Worker *worker);

#endif
