/* called on start/stop ... */
#ifndef _WORKER_H
#define _WORKER_H

#include <pthread.h>

#include "libcx-list/list.h"
#include "libcx-base/ev.h"

typedef enum worker_event_t
{
	WORKER_EVENT_START,
	/* stop connection watcher ... */
	WORKER_EVENT_STOP,
	/* worker has left the event loop  - resources can be released */
	WORKER_EVENT_RELEASE,
} WorkerEvent;

typedef struct worker_t Worker;
typedef void F_WorkerEventHandler (Worker *worker, WorkerEvent event, void *data);

struct worker_t
{
	/* statistics ? */
	unsigned long id;                       /* the worker id */
	pthread_t *thread;                      /* the worker thread */
	ev_loop *loop;                          /* the workers event loop */
	F_WorkerEventHandler *f_handler;        /* generic worker event handler */
};

typedef struct unix_worker_t
{
	Worker *worker;
	int server_fd;
	ev_io *connection_watcher;
	List *requests;                 /* pending requests */

} UnixWorker;

Worker*
Worker_new(unsigned long id);

void
Worker_free(Worker *worker);

int
Worker_start(Worker *worker);

void
Worker_stop(Worker *worker);



#endif
