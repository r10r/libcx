#include "queue.h"

Queue*
Queue_new()
{
	Queue *queue = malloc(sizeof(Queue));

	queue->mutex_add_item = malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(queue->mutex_add_item, NULL);

	queue->mutex_cond_add_item = malloc(sizeof(pthread_cond_t));
	pthread_cond_init(queue->mutex_cond_add_item, NULL);

	queue->items = List_new();

	queue->_active = true;

	return queue;
}

void
Queue_free(Queue *queue)
{
	List_free(queue->items);

	pthread_mutex_destroy(queue->mutex_add_item);
	free(queue->mutex_add_item);

	pthread_cond_destroy(queue->mutex_cond_add_item);
	free(queue->mutex_cond_add_item);

	free(queue);
}

// POP returns NULL when Queue is not active anymore ?
// -> check for null in the calling thread
void*
Queue_pop(Queue *queue)
{
	void *data = NULL;
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

int
Queue_add(Queue *queue, void *data)
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
Queue_destroy(Queue *queue)
{
	int rc = 0;

	printf("Queue is destroyed\n");

	queue->_active = false;

	/* wakeup up all waiting threads */
	rc = pthread_cond_broadcast(queue->mutex_cond_add_item);
}


