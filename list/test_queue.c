#include <unistd.h>
#ifdef __linux__
#include <signal.h>
#endif

#include "../base/test.h"
#include "queue.h"

/*
 * Observations:
 * 90% of the time is spend in for acquiring and releasing the lock
 * as well as signaling / waiting for the condition
 *
 */

#define NTHREADS 4
#define NITERATATIONS 100000

typedef struct consumer_t
{
	Queue* queue;
	int id;
	pthread_t* thread;
	int processed;
} Consumer;

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

	while (Queue_active(consumer->queue))
	{
		int* x = Queue_pop_wait(consumer->queue);

		// last iteration might return NULL
		if (x)
		{
			int i = *x;
			cx_free(x);
			consumer->processed++;

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
Consumer_start(Queue* queue, int id)
{
	Consumer* consumer = Consumer_new(queue, id);

	pthread_create(consumer->thread, NULL, start_consumer, consumer);
	return consumer;
}

static void
test_Queue()
{
	Queue* queue = Queue_new();

	Consumer* consumers[NTHREADS];

	int i_thread;

	for (i_thread = 0; i_thread < NTHREADS; i_thread++)
		consumers[i_thread] = Consumer_start(queue, i_thread);


	PROFILE_BEGIN_FMT("Processing %d simple requests with %d threads\n",
			  NITERATATIONS, NTHREADS);

	int i_item;
	int expected_sum = 0;
	for (i_item = 0; i_item < NITERATATIONS; i_item++)
	{
		int* x = cx_alloc(sizeof(int));
		*x = i_item;
		Queue_add(queue, x);
		expected_sum += i_item;
	}

	// wake up all threads and tell them to finish up
	int total_processed = 0;
	for (i_thread = 0; i_thread < NTHREADS; i_thread++)
	{
		Consumer* consumer = consumers[i_thread];
		// see http://stackoverflow.com/questions/5610677/valgrind-memory-leak-errors-when-using-pthread-create
		//	http://stackoverflow.com/questions/5282099/signal-handling-in-pthreads
		pthread_join(*consumer->thread, NULL);
		printf("Consumer[%d] processed %d\n", consumer->id, consumer->processed);
		total_processed += consumer->processed;
		Consumer_free(consumer);
	}

	PROFILE_END

	TEST_ASSERT_EQUAL(total_processed, NITERATATIONS);

	Queue_free(queue);
}

int
main()
{
	TEST_BEGIN

	RUN(test_Queue);

	TEST_END
}
