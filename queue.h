#ifndef _QUEUE_H
#define _QUEUE_H

#include <stdio.h>
#include <pthread.h>
#define _LIST_DISABLE_LOCKING
#include "list.h"

typedef struct queue_t
{
	List *items;
	pthread_cond_t *mutex_cond_add_item;
	pthread_mutex_t *mutex_add_item;
} Queue;

Queue*
Queue_new(void);

void
Queue_free(Queue *queue);

void*
Queue_pop(Queue *queue);

void
Queue_add(Queue *queue, void *data);

#endif
