#ifndef _WORKER_H
#define _WORKER_H

#include <pthread.h>

#include "libcx-list/list.h"
#include "libcx-base/ev.h"
#include "connection.h"

typedef enum worker_event_t
{
	WORKER_EVENT_NEW,               /* create a new worker */
	WORKER_EVENT_START,
	/* stop connection watcher ... */
	WORKER_EVENT_STOP,
	/* worker has left the event loop  - resources can be released */
	WORKER_EVENT_RELEASE,
} WorkerEvent;

typedef struct worker_t Worker;
typedef Worker* F_WorkerHandler (Worker *worker, WorkerEvent event);

struct worker_t
{
	/* statistics ? */
	unsigned long id;               /* the worker id */
	pthread_t *thread;              /* the worker thread */
	ev_loop *loop;                  /* the workers event loop */

	F_WorkerHandler *f_handler;     /* generic worker event handler */
	F_ConnectionHandler *f_connection_handler;
};

void
Worker_new(Worker *worker);

void
Worker_free(Worker *worker);

int
Worker_start(Worker *worker);

void
Worker_stop(Worker *worker);

#endif
