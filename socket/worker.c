/* FIXME circular inclusion */
#include "server.h"
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
	worker->loop = ev_loop_new(EVBACKEND);
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
	int rc = pthread_create(worker->thread, NULL, _Worker_run, worker);

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
		XFDBG("Worker[%lu] exiting no handler", worker->id);
	return NULL;
}

void
Worker_stop(Worker* worker)
{
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
