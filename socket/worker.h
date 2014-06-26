#ifndef _CX_WORKER_H
#define _CX_WORKER_H

#include <pthread.h>

#include "../list/list.h"
#include "server.h"

#ifdef PTHREAD_STACK_MIN
#define WORKER_THREAD_STACK_SIZE PTHREAD_STACK_MIN * 2
#else
/* FIXME only on OSX __DARWIN_C_LEVEL not defined properly ? */
#define WORKER_THREAD_STACK_SIZE 8192 * 2
#endif


typedef struct cx_worker_t Worker;
typedef void F_WorkerHandler (Worker* worker);

struct cx_worker_t
{
	/* statistics ? */
	unsigned long id;               /* the worker id */
	pthread_t* thread;              /* the worker thread */
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
