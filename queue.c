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

void*
Queue_pop(Queue *queue)
{
	void *data;
	int rc = 0;

	rc = pthread_mutex_lock(queue->mutex_add_item);
	XFCHECK(rc == 0,
		"pthread_mutex_lock should exit with 0 (was %d)", rc);

	while (queue->items->length == 0)
		rc = pthread_cond_wait(queue->mutex_cond_add_item, queue->mutex_add_item);
	XFCHECK(rc == 0,
		"pthread_cond_wait should exit with 0 (was %d)", rc);

	data = List_pop(queue->items);

	pthread_mutex_unlock(queue->mutex_add_item);
	XFCHECK(rc == 0,
		"pthread_mutex_unlock should exit with 0 (was %d)", rc);
	return data;
}

void
Queue_add(Queue *queue, void *data)
{
	int rc = 0;

	// sender (task was added to queue)
	pthread_mutex_lock(queue->mutex_add_item);
	XFCHECK(rc == 0,
		"pthread_mutex_lock should exit 0 (was %d)", rc);

	List_unshift(queue->items, data);
	// wakes up one thread waiting for condition &cond
	// much slower for a lot of wakeups
	//	int ret = pthread_cond_broadcast(queue->mutex_cond_add_item);
	int ret = pthread_cond_signal(queue->mutex_cond_add_item);
	XFCHECK(rc == 0,
		"pthread_cond_signal should exit with 0 (was %d)", rc);

	pthread_mutex_unlock(queue->mutex_add_item);
	XFCHECK(rc == 0,
		"pthread_mutex_unlock should exit 0 (was %d)", rc);
}
