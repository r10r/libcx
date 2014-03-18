#include "libcx-base/test.h"
#include "queue.h"
#include <unistd.h>

/*
 * Observations:
 * 90% of the time is spend in for acquiring and releasing the lock
 * as well as signaling / waiting for the condition
 *
 */
NOSETUP
PROFILE_INIT

static int checksum = 0;
static pthread_mutex_t mutex_checksum;
static pthread_mutex_t mutex_done;

#define NTHREADS 4
#define NITERATATIONS 10000

static void
thread_wait(int nanos)
{
	int sleep_nanos = nanos;
	struct timespec sleep_time;

	sleep_time.tv_sec = 0;
	sleep_time.tv_nsec = sleep_nanos;
	nanosleep(&sleep_time, NULL);
}

static void
checksum_add(int i)
{
	pthread_mutex_lock(&mutex_checksum);
	checksum += i;
	pthread_mutex_unlock(&mutex_checksum);
}

typedef struct consumer_t
{
	Queue *queue;
	int id;
	pthread_t *thread;
	int consumed;
} Consumer;


static Consumer *
Consumer_new(Queue *queue, int id)
{
	Consumer *consumer = malloc(sizeof(Consumer));

	consumer->id = id;
	consumer->queue = queue;
	consumer->thread = malloc(sizeof(pthread_t));
	consumer->consumed = 0;
	return consumer;
}

static void*
start_consumer(void *data)
{
	Consumer *consumer = (Consumer*)data;

	while (true)
	{
		int *x = Queue_pop(consumer->queue);
		int i = *x;
		free(x);
		consumer->consumed++;

//		printf("[%d] Consumed %d\n", consumer->id, i);
		// FIXME benchmark is faster with logging (~ 15%)
		// logging should have a major impact on the performance
		// I assume it introduces an delay which helps when
		// calling pthread_cond_signal
		printf("[%d] Consumed %d\n", consumer->id, i);

		// thread wait does not have the same impact
//		thread_wait(i);
		checksum_add(i);

		if ((NITERATATIONS - 1) == i)
		{
			PROFILE_END
			pthread_mutex_unlock(&mutex_done);
			break;
		}
	}

	return NULL;
}

static Consumer *
Consumer_start(Queue *queue, int id)
{
	Consumer *consumer = Consumer_new(queue, id);

	pthread_create(consumer->thread, NULL, start_consumer, consumer);
	return consumer;
}

static void
Consumer_free(Consumer *consumer)
{
	free(consumer->thread);
	free(consumer);
}

static void
test_Queue()
{
	Queue *queue = Queue_new();

	Consumer* consumers[NTHREADS];

	int i_thread;

	for (i_thread = 0; i_thread < NTHREADS; i_thread++)
		consumers[i_thread] = Consumer_start(queue, i_thread);


	pthread_mutex_init(&mutex_checksum, NULL);
	pthread_mutex_init(&mutex_done, NULL);
	pthread_mutex_lock(&mutex_done);

	PROFILE_BEGIN_FMT("Processing %d simple requests with %d threads\n",
			  NITERATATIONS, NTHREADS);

	int i_item;
	int expected_sum = 0;
	for (i_item = 0; i_item < NITERATATIONS; i_item++)
	{
		int *x = malloc(sizeof(int));
		*x = i_item;
		Queue_add(queue, x);
		expected_sum += i_item;
	}

	// wait until done
	pthread_mutex_lock(&mutex_done);

	TEST_ASSERT_EQUAL(expected_sum, checksum);

	for (i_thread = 0; i_thread < NTHREADS; i_thread++)
	{
		Consumer *consumer = consumers[i_thread];
		printf("Consumer[%d] consumed %d\n", consumer->id, consumer->consumed);
		Consumer_free(consumer);
	}

	pthread_mutex_destroy(&mutex_checksum);
	pthread_mutex_destroy(&mutex_done);
	Queue_free(queue);
}

int
main()
{
	TEST_BEGIN

	RUN(test_Queue);

	TEST_END
}
