#include "queue.h"

#define __RWLOCK_LOCK_READ(rwlock) \
	{ cx_assert(pthread_rwlock_rdlock(rwlock) == 0);

#define __RWLOCK_LOCK_WRITE(rwlock) \
	{ cx_assert(pthread_rwlock_wrlock(rwlock) == 0);

#define __RWLOCK_UNLOCK(rwlock) \
	cx_assert(pthread_rwlock_unlock(rwlock) == 0); }

/*
   #define __LOCK(mutex) \
        { cx_assert(pthread_mutex_lock(mutex) == 0);

   #define __UNLOCK(mutex) \
        cx_assert(pthread_mutex_unlock(mutex) == 0); }
 */

Queue*
Queue_new()
{
	Queue* queue = cx_alloc(sizeof(Queue));

	List_init(((List*)queue));

	pthread_rwlock_init(&queue->rwlock, NULL);
	pthread_mutex_init(&queue->item_added_mutex, NULL);
	pthread_cond_init(&queue->item_added_condition, NULL);

	queue->_active = true;

	return queue;
}

#include <unistd.h> /* sleep */

void
Queue_destroy(Queue* queue)
{
	assert(queue->_active);
	XDBG("Queue is destroyed");
	queue->_active = false;

	/* wakeup up all waiting threads */
	cx_assert(pthread_cond_broadcast(&queue->item_added_condition) == 0);

	pthread_mutex_destroy(&queue->item_added_mutex);
	pthread_cond_destroy(&queue->item_added_condition);

	cx_assert(pthread_rwlock_wrlock(&queue->rwlock) == 0);
	List_free_members((List*)queue);

	pthread_rwlock_rdlock(&queue->rwlock);
	pthread_rwlock_destroy(&queue->rwlock);

	cx_free(queue);
}

/*
 * @return
 * -1 when queue is inactive
 *	0 when queue is locked
 *	1 when data was retrieved successfully from the queue
 */
int
Queue_shift(Queue* queue, void** data)
{
	if (!Queue_active(queue))
		return -1;

	int locked = pthread_rwlock_wrlock(&queue->rwlock);
	if (locked == 0)
	{
		int ret = -1;

		/* since we have aquired the lock before the queue
		 * becomes inactive we continue processing
		 */
		void* node_data = List_shift((List*)queue);
		if (node_data)
		{
			*data = node_data;
			ret = 1;
		}
		else
			ret = 0;

		cx_assert(pthread_rwlock_unlock(&queue->rwlock) == 0);
		return ret;
	}
	else
	{
		XFDBG("LOCK status %s", strerror(locked));
		return -1;
	}
}

inline int
Queue_get(Queue* queue, void** data)
{
	return Queue_shift(queue, data);
}

static void
timeout_nanos(struct timespec* ts, int wait_nanos)
{
	struct timeval tv;

	cx_assert(gettimeofday(&tv, NULL) == 0);

	TIMEVAL_TO_TIMESPEC(&tv, ts);

	long tv_sec = ts->tv_sec + (wait_nanos / BILLION);
	long tv_nsec = ts->tv_nsec + (wait_nanos % BILLION);

	/* set timeout values and correct overflow of tv_nsec */
	ts->tv_sec = tv_sec + (tv_nsec / BILLION);
	ts->tv_nsec = tv_nsec % BILLION;
}

int
Queue_get_timedwait(Queue* queue, void** data, int wait_nanos)
{
	/* check if queue has data which we can get right away */
	int have_item = Queue_get(queue, data);

	/* break here if queue is inactive or we have already retrieved an item */
	if (have_item == -1 || have_item == 1)
		return have_item;

	/*
	 * Otherwise wait for new items.
	 * If mutex was destroyed right before calling pthread_mutex_lock
	 * pthread_mutex_lock must return EINVAL.
	 */
	int mutex_status = pthread_mutex_lock(&queue->item_added_mutex);

	if (mutex_status == 0)
	{
		/*
		 * Check if queue has been destroyed before waiting.
		 * If we don't do this we have to check for an error response code
		 * from pthread_cond_*wait.
		 */
		if (Queue_active(queue))
		{
			if (wait_nanos <= 0)
			{
				cx_assert(pthread_cond_wait(&queue->item_added_condition, &queue->item_added_mutex) == 0);
			}
			else
			{
				int rc = 0;
				struct timespec ts;
				timeout_nanos(&ts, wait_nanos);
				rc = pthread_cond_timedwait(&queue->item_added_condition, &queue->item_added_mutex, &ts);
				cx_assert(rc == 0 || rc == ETIMEDOUT);
			}

			int wait_have_item = -1;
			/* check if queue has been destroyed after waiting */
			if (Queue_active(queue))
				wait_have_item = Queue_get(queue, data);

			pthread_mutex_unlock(&queue->item_added_mutex);
			return wait_have_item;
		}
		else
		{
			/* queue is not active any longer */
			cx_assert(pthread_mutex_unlock(&queue->item_added_mutex) == 0);
			return -1;
		}
	}
	else
	{
		XFERR("Failed to aquire item_added_mutex : %s", strerror(mutex_status));
		return -1;
	}
}

inline int
Queue_get_wait(Queue* queue, void** data)
{
	return Queue_get_timedwait(queue, data, 0);
}

int
Queue_push(Queue* queue, void* data)
{
	if (!Queue_active(queue))
		return -1;

	__RWLOCK_LOCK_WRITE(&queue->rwlock)

	List_push((List*)queue, data);

	__RWLOCK_UNLOCK(&queue->rwlock)

	// wake up a single thread that is waiting for the condition
	cx_assert(pthread_cond_signal(&queue->item_added_condition) == 0);

	return 0;
}

inline int
Queue_add(Queue* queue, void* data)
{
	return Queue_push(queue, data);
}

int
Queue_each(Queue* queue, F_NodeIterator* f_node_iterator, void* userdata)
{
	if (!Queue_active(queue))
		return -1;

	__RWLOCK_LOCK_READ(&queue->rwlock)
	List_each((List*)queue, f_node_iterator, userdata);
	__RWLOCK_UNLOCK(&queue->rwlock)
	return 1;
}

int
Queue_match_node(Queue* queue, const void* key, void** data)
{
	if (!Queue_active(queue))
		return -1;

	int item_matched = -1;

	__RWLOCK_LOCK_READ(&queue->rwlock)
	void* node_data = List_match_node((List*)queue, key);
	if (node_data)
	{
		*data = node_data;
		item_matched = 1;
	}
	else
		item_matched = 0;
	__RWLOCK_UNLOCK(&queue->rwlock)

	return item_matched;
}

//int
//Queue_delete(Queue*queue, unsigned int index)
//{
//	if (!Queue_active(queue))
//		return -1;
//
//	int item_deleted = -1;
//	__RWLOCK_LOCK_WRITE(&queue->rwlock)
//	List_delete((List*)queue,  index);
//	__RWLOCK_UNLOCK(&queue->rwlock)
//
//	return 1;
//}

//int
//Queue_at(Queue* queue, unsigned int index, Node *node)
//{
//	if (!Queue_active(queue))
//		return -1;
//
//	int locked = pthread_rwlock_rdlock(&queue->rwlock);
//	if (locked == 0)
//	{
//		int ret = -1;
//
//		/* check if queue has been destroyed meanwhile */
//		if (Queue_active(queue))
//		{
//			// FIXME must clone entry here
//			node = List_at((List*)queue);
//			if (node)
//			{
//				ret = 1;
//			}
//			else
//				ret = 0;
//		}
//
//		cx_assert(pthread_rwlock_unlock(&queue->rwlock) == 0);
//		return ret;
//	}
//	else
//	{
//		XFDBG("LOCK status %s", strerror(locked));
//		return -1;
//	}
//}
