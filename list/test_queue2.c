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
#define NITERATATIONS 100
#define SUBMISSION_DELAY_MAX_MSEC 10    /* ever nth mseconds an item is submitted */
#define PROCESSING_DELAY_MAX_MSEC 100

typedef struct consumer_t
{
	Queue* queue;
	int id;
	pthread_t* thread;
	int matched;
} Consumer;

static Consumer*
Consumer_new(Queue* queue, int id)
{
	Consumer* consumer = cx_alloc(sizeof(Consumer));

	consumer->id = id;
	consumer->queue = queue;
	consumer->thread = cx_alloc(sizeof(pthread_t));
	consumer->matched = 0;
	return consumer;
}

static void
Consumer_free(Consumer* consumer)
{
	cx_free(consumer->thread);
	cx_free(consumer);
}

static long
match_random_number(Node* node, const void* keydata)
{
	int node_number = *((int*)(node->data));
	int random_number = *((const int*)keydata);

	if (node_number == random_number)
	{
		XFDBG("match expected:%d, actual:%d", random_number, node_number);
		return 0;
	}
	else
		return -1;
}

static int
node_iterator(int index, Node* node, void* userdata)
{
	UNUSED(index);
	UNUSED(node);
	UNUSED(userdata);
	; // noop
	return 1;
}

static void*
start_consumer(void* data)
{
	Consumer* consumer = (Consumer*)data;

	int* value = NULL;

#ifdef PROCESSING_DELAY_MAX_MSEC
	usleep((rand() % PROCESSING_DELAY_MAX_MSEC) * 1000);
#endif

	int key =  lrand48() % NITERATATIONS;
	XFDBG("key: %d", key);

	while (true)
	{
		int match_status = Queue_match_node(consumer->queue, (const void*)&key, (void**)&value);
		/* simply iterate over all elements */
		(void)Queue_each(consumer->queue, &node_iterator, NULL);

		if (match_status == 0)
			continue;

		if (match_status == 1)
		{
			XFDBG("Consumer[%d] matched value %d", consumer->id, *value);
			consumer->matched++;
			break;
		}
		else if (match_status == -1)
		{
			XFDBG("Consumer[%d] - queue inactive", consumer->id);
			break;
		}
	}

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

	((List*)queue)->f_node_data_compare = match_random_number;

	Consumer* consumers[NTHREADS_TOTAL];

	int i_thread;

	for (i_thread = 0; i_thread < (NTHREADS_TOTAL); i_thread++)
		consumers[i_thread] = Consumer_start(queue, i_thread);

	PROFILE_BEGIN_FMT("Processing %d simple requests with %d threads\n",
			  NITERATATIONS, NTHREADS_TOTAL);

	int i_item;
	int expected_sum = 0;

	for (i_item = 0; i_item < NITERATATIONS; i_item++)
	{
		int* x = cx_alloc(sizeof(int));
		*x = i_item;
		Queue_add(queue, x);
		expected_sum += i_item;
#ifdef SUBMISSION_DELAY_MAX_MSEC
		usleep((rand() % SUBMISSION_DELAY_MAX_MSEC) * 1000);
#endif
	}

	/* wait for threads to finish processing */
	int num_matched = 0;
	for (i_thread = 0; i_thread < NTHREADS_TOTAL; i_thread++)
	{
		Consumer* consumer = consumers[i_thread];
		pthread_join(*consumer->thread, NULL);
		XFLOG("Consumer[%d] processed %d", consumer->id, consumer->matched);
		num_matched += consumer->matched;
		Consumer_free(consumer);
	}

	Queue_destroy(queue);


	PROFILE_END

	TEST_ASSERT_EQUAL(num_matched, NITERATATIONS);
}

int
main()
{
	TEST_BEGIN

	RUN(test_Queue);

	TEST_END
}
