#include <unistd.h>
#ifdef __linux__
#include <signal.h>
#endif

#define _CX_PROFILE
#include <libcx/base/test.h>
#include "queue.h"

/*
 * Observations:
 * 90% of the time is spend in for acquiring and releasing the lock
 * as well as signaling / waiting for the condition
 */
#define NTHREADS_TOTAL 100
#define NITERATATIONS 1000 * 10
//#define SUBMISSION_DELAY_MAX_MSEC 10
#define PROCESSING_DELAY_MAX_MSEC 10

typedef int F_GetItem (Queue* queue, int** int_ptr);

typedef struct consumer_t
{
	Queue* queue;
	int id;
	pthread_t* thread;
	int processed;
	F_GetItem* f_get_item;
} Consumer;

static int
get_item(Queue* queue, int** x)
{
//	return Queue_get(queue, (void**)x);
	return Queue_get_wait(queue, (void**)x);
//	return Queue_get_timedwait(queue, (void**)x, 100 * 1000);
}

static int
get_item_timed(Queue* queue, int** x)
{
//	return Queue_get(queue, (void**)x);
//	return Queue_get_wait(queue, (void**)x);
	return Queue_get_timedwait(queue, (void**)x, 100 * 1000);
}

static Consumer*
Consumer_new(Queue* queue, int id)
{
	Consumer* consumer = cx_alloc(sizeof(Consumer));

	consumer->id = id;
	consumer->queue = queue;
	consumer->thread = cx_alloc(sizeof(pthread_t));
	consumer->processed = 0;
	return consumer;
}

static void
Consumer_free(Consumer* consumer)
{
	cx_free(consumer->thread);
	cx_free(consumer);
}

static void*
start_consumer(void* data)
{
	Consumer* consumer = (Consumer*)data;

	int* x;
	int has_item = 0;

	while ((has_item = consumer->f_get_item(consumer->queue, &x)) != -1)
	{
		if (has_item == 1)
		{
			int i = *x;
			cx_free(x);
			consumer->processed++;

			XFLOG("Consumer[%d] - received %d", consumer->id, i);

			/* simulate processing delay */
#ifdef PROCESSING_DELAY_MAX_MSEC
			usleep((rand() % PROCESSING_DELAY_MAX_MSEC) * 1000);
#endif

			// queue is destroyed on the last request
			if (i == (NITERATATIONS - 1))
			{
				XFDBG("Consumer[%d] I'm processing the last request", consumer->id);
				Queue_destroy(consumer->queue);
			}
		}
	}
	XFDBG("Consumer[%d] Leaving inactive queue", consumer->id);
	pthread_exit(NULL);
}

static Consumer*
Consumer_start(Queue* queue, int id, F_GetItem* f_get_item)
{
	Consumer* consumer = Consumer_new(queue, id);

	consumer->f_get_item = f_get_item;
	pthread_create(consumer->thread, NULL, start_consumer, consumer);
	return consumer;
}

static void
test_Queue()
{
	Queue* queue = Queue_new();

	Consumer* consumers[NTHREADS_TOTAL];

	int i_thread;

	for (i_thread = 0; i_thread < (NTHREADS_TOTAL / 2); i_thread++)
		consumers[i_thread] = Consumer_start(queue, i_thread, get_item);

	for (; i_thread < NTHREADS_TOTAL; i_thread++)
		consumers[i_thread] = Consumer_start(queue, i_thread, get_item_timed);

	PROFILE_BEGIN_FMT("Processing %d simple requests with %d threads\n",
			  NITERATATIONS, NTHREADS_TOTAL);

	int i_item;
	int expected_sum = 0;

	for (i_item = 0; i_item < NITERATATIONS; i_item++)
	{
		int* x = cx_alloc(sizeof(int));
		*x = i_item;
		Queue_add(queue, x);
		/* simulate submission delay */

		expected_sum += i_item;

#ifdef SUBMISSION_DELAY_MAX_MSEC
		usleep((rand() % SUBMISSION_DELAY_MAX_MSEC) * 1000);
#endif
	}

	/* wait for threads to finish processing */
	int total_processed = 0;
	for (i_thread = 0; i_thread < NTHREADS_TOTAL; i_thread++)
	{
		Consumer* consumer = consumers[i_thread];
		// see http://stackoverflow.com/questions/5610677/valgrind-memory-leak-errors-when-using-pthread-create
		//	http://stackoverflow.com/questions/5282099/signal-handling-in-pthreads
		pthread_join(*consumer->thread, NULL);
		XFLOG("Consumer[%d] processed %d", consumer->id, consumer->processed);
		total_processed += consumer->processed;
		Consumer_free(consumer);
	}

	PROFILE_END

	TEST_ASSERT_EQUAL(total_processed, NITERATATIONS);
}

int
main()
{
	TEST_BEGIN

	RUN(test_Queue);

	TEST_END
}
