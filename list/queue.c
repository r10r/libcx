#include "queue.h"

Queue*
Queue_new()
{
	Queue* queue = cx_alloc(sizeof(Queue));

	queue->mutex_add_item = cx_alloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(queue->mutex_add_item, NULL);

	queue->mutex_cond_add_item = cx_alloc(sizeof(pthread_cond_t));
	pthread_cond_init(queue->mutex_cond_add_item, NULL);

	queue->items = List_new();

	queue->_active = true;

	return queue;
}

void
Queue_free(Queue* queue)
{
	List_free(queue->items);

	pthread_mutex_destroy(queue->mutex_add_item);
	cx_free(queue->mutex_add_item);

	pthread_cond_destroy(queue->mutex_cond_add_item);
	cx_free(queue->mutex_cond_add_item);

	cx_free(queue);
}

/* TODO test */
void*
Queue_pop(Queue* queue)
{
	void* data = NULL;
	int rc = 0;

	rc = pthread_mutex_lock(queue->mutex_add_item);
	XFCHECK(rc == 0,
		"pthread_mutex_lock should exit with 0 (was %d)", rc);

	/*
	 * When the queue was destroyed/deactivated, multiple threads
	 * access this region concurrently. To ensure
	 * that only a single thread calls List_pop we have
	 * to check whether the queue is active.
	 */
	if (Queue_active(queue))
		data = List_pop(queue->items);

	pthread_mutex_unlock(queue->mutex_add_item);
	XFCHECK(rc == 0,
		"pthread_mutex_unlock should exit with 0 (was %d)", rc);
	return data;
}

// TODO add timeout parameter using pthread_cond_timed_wait
// add synchronized method without waiting

// POP returns NULL when Queue is not active anymore ?
// -> check for null in the calling thread
void*
Queue_pop_wait(Queue* queue)
{
	void* data = NULL;
	int rc = 0;

	rc = pthread_mutex_lock(queue->mutex_add_item);
	XFCHECK(rc == 0,
		"pthread_mutex_lock should exit with 0 (was %d)", rc);

	while (Queue_active(queue) && queue->items->length == 0)
	{
		rc = pthread_cond_wait(queue->mutex_cond_add_item, queue->mutex_add_item);
		XFCHECK(rc == 0,
			"pthread_cond_wait should exit with 0 (was %d)", rc);
	}

	/*
	 * When the queue was destroyed/deactivated, multiple threads
	 * access this region concurrently. To ensure
	 * that only a single thread calls List_pop we have
	 * to check whether the queue is active.
	 */
	if (Queue_active(queue))
		data = List_pop(queue->items);

	pthread_mutex_unlock(queue->mutex_add_item);
	XFCHECK(rc == 0,
		"pthread_mutex_unlock should exit with 0 (was %d)", rc);
	return data;
}

/* TODO test */
void*
Queue_pop_timedwait(Queue* queue, long wait_nanos)
{
	void* data = NULL;
	int rc = 0;

	rc = pthread_mutex_lock(queue->mutex_add_item);
	XFCHECK(rc == 0,
		"pthread_mutex_lock should exit with 0 (was %d)", rc);

	while (Queue_active(queue) && queue->items->length == 0)
	{
		struct timespec ts;
		struct timeval tv;
		rc = gettimeofday(&tv, NULL);

		if (rc != 0)
		{
			XFDBG("error gettimeofday : %s", strerror(rc));
			exit(1);
		}

		TIMEVAL_TO_TIMESPEC(&tv, &ts);

		long tv_sec = ts.tv_sec + (wait_nanos / BILLION);
		long tv_nsec = ts.tv_nsec + (wait_nanos % BILLION);

		/* set timeout values and correct overflow of tv_nsec */
		ts.tv_sec = tv_sec + (tv_nsec / BILLION);
		ts.tv_nsec = tv_nsec % BILLION;

		rc = pthread_cond_timedwait(queue->mutex_cond_add_item, queue->mutex_add_item, &ts);
		XFCHECK(rc == 0,
			"pthread_cond_wait should exit with 0 (was %d)", rc);
	}

	/*
	 * When the queue was destroyed/deactivated, multiple threads
	 * access this region concurrently. To ensure
	 * that only a single thread calls List_pop we have
	 * to check whether the queue is active.
	 */
	if (Queue_active(queue))
		data = List_pop(queue->items);

	pthread_mutex_unlock(queue->mutex_add_item);
	XFCHECK(rc == 0,
		"pthread_mutex_unlock should exit with 0 (was %d)", rc);
	return data;
}

int
Queue_add(Queue* queue, void* data)
{
	if (!Queue_active(queue))
		return -1;

	int rc = 0;

	// sender (task was added to queue)
	pthread_mutex_lock(queue->mutex_add_item);
	XFCHECK(rc == 0,
		"pthread_mutex_lock should exit 0 (was %d)", rc);

	List_unshift(queue->items, data);
	// wake up a single thread that is waiting for the condition
	rc = pthread_cond_signal(queue->mutex_cond_add_item);
	XFCHECK(rc == 0,
		"pthread_cond_signal should exit with 0 (was %d)", rc);

	pthread_mutex_unlock(queue->mutex_add_item);
	XFCHECK(rc == 0,
		"pthread_mutex_unlock should exit 0 (was %d)", rc);

	return rc;
}

void
Queue_destroy(Queue* queue)
{
	int rc = 0;

	XDBG("Queue is destroyed");

	queue->_active = false;

	/* wakeup up all waiting threads */
	rc = pthread_cond_broadcast(queue->mutex_cond_add_item);

	XFCHECK(rc == 0,
		"pthread_cond_broadcast should exit with 0 (was %d)", rc);
}
