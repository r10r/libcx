#ifndef _CX_QUEUE_H
#define _CX_QUEUE_H

#include <stdio.h>
#include <pthread.h>
#include <stdbool.h>

#include "../list/list.h"

typedef struct cx_queue_t
{
	List* items;
	pthread_cond_t* mutex_cond_add_item;
	pthread_mutex_t* mutex_add_item;
	bool _active;   // TODO hide
} Queue;

#define Queue_active(queue) \
	queue->_active

Queue*
Queue_new(void);

void
Queue_free(Queue* queue);

void*
Queue_pop(Queue* queue);

/* 0 when the item was queued */
int
Queue_add(Queue* queue, void* data);

// wake up all waiting workers so they can check whether to exit or not
/* block insert signal all waiting workers that the queue is going to be destroyed */
void
Queue_destroy(Queue* queue);

#endif
