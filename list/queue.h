#ifndef _CX_QUEUE_H
#define _CX_QUEUE_H

#include <stdio.h>
#include <pthread.h>
#include <stdbool.h>
#include <sys/time.h> /* gettimeofday */

#include "list.h"

typedef struct cx_queue_t
{
	List items;
	pthread_mutex_t item_added_mutex;
	pthread_cond_t item_added_condition;
	pthread_rwlock_t rwlock;
	bool _active;   // TODO hide
} Queue;

#define Queue_active(queue) \
	queue->_active

Queue*
Queue_new(void);

// wake up all waiting workers so they can check whether to exit or not
/* block insert signal all waiting workers that the queue is going to be destroyed */
void
Queue_destroy(Queue* queue);

int
Queue_get(Queue* queue, void** data);

int
Queue_get_wait(Queue* queue, void** data);

int
Queue_get_timedwait(Queue* queue, void** data, int wait_nanos);

/* 0 when the item was queued */
int
Queue_add(Queue* queue, void* data);


#endif
