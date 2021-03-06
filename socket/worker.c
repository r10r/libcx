#include "worker.h"

static void*
_Worker_run(void* data);

Worker*
Worker_new()
{
	Worker* worker = cx_alloc(sizeof(Worker));

	Worker_init(worker);
	return worker;
}

void
Worker_init(Worker* worker)
{
	worker->thread = cx_alloc(sizeof(pthread_t));
	worker->f_handler = NULL;
}

void
Worker_free(Worker* worker)
{
	cx_free(worker->thread);
	cx_free(worker);
}

/* @return 0 when worker was started sucessfully */
int
Worker_start(Worker* worker)
{
	pthread_attr_t attr;

	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, WORKER_THREAD_STACK_SIZE);
	int rc = pthread_create(worker->thread, &attr, _Worker_run, worker);
//	int rc = pthread_create(worker->thread, NULL, _Worker_run, worker);

	if ( rc != 0 )
	{
		fprintf(stderr, "Failed to start worker[%lu]: Error %d\n", worker->id, rc);
		return -1;
	}
	return 0;
}

static void*
_Worker_run(void* data)
{
	Worker* worker = (Worker*)data;

	XFDBG("Worker[%lu] started", worker->id);
	if (worker->f_handler)
		worker->f_handler(worker);
	else
		XFWARN("Worker[%lu] exiting no handler", worker->id);
	return NULL;
}

void
Worker_stop(Worker* worker)
{
	UNUSED(worker);

	// TODO send async signal to worker to finish processing ?

	XDBG("Not implemented");
	// FIXME must signal worker to close all connections (using async_send ?)
//	XASSERT("Task list should be empty", worker->tasks->length == 0)
//	XASSERT("No tasks should be in progress")
	// TODO wait for all tasks to cancel, ....
//	FIXME terminate worker properly check that all connections are closed
	// pthread_kill, pthread_cancel
//			http://stackoverflow.com/questions/18826853/how-to-stop-a-running-pthread-thread
//	pthread_cancel(worker->thread);
}
