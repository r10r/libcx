#include "libcx-base/debug.h"
#include "worker.h"

// ev_timer *manage_tasks_watcher;	/* expire tasks, handover tasks ... */

// TODO howto block list until work is available ?

static void*
_worker_init(void *data)
{
	Worker *worker = (Worker*)data;

	worker->loop = ev_loop_new(0);
	ev_set_userdata(worker->loop, worker);

	/* call event handler to handle custom initialization */
	worker->f_handler(worker, WORKER_EVENT_START, NULL);
	printf("Worker[%lu] started\n", worker->id);

	ev_run(worker->loop, 0);
	return NULL;
}

Worker*
Worker_new(unsigned long id)
{
	Worker *worker = malloc(sizeof(Worker));

	worker->id = id;
	worker->thread = malloc(sizeof(pthread_t));
	return worker;
}

void
Worker_free(Worker *worker)
{
	free(worker->thread);
	free(worker);
}

/* 0 when worker was started sucessfully */
int
Worker_start(Worker *worker)
{
	worker->thread = malloc(sizeof(pthread_t));
	int rc = pthread_create(worker->thread, NULL, _worker_init, worker);
	if ( rc != 0 )
	{
		fprintf(stderr, "Failed to start worker[%lu]: Error %d\n", worker->id, rc);
		return -1;
	}
	return 0;
}

void
Worker_stop(Worker *worker)
{
	worker->f_handler(worker, WORKER_EVENT_STOP, NULL);
//	XASSERT("Task list should be empty", worker->tasks->length == 0)
//	XASSERT("No tasks should be in progress")
	// TODO wait for all tasks to cancel, ....
//	FIXME terminate worker properly check that all connections are closed
	// pthread_kill, pthread_cancel
//			http://stackoverflow.com/questions/18826853/how-to-stop-a-running-pthread-thread
//	pthread_cancel(worker->thread);
}

