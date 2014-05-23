#include <pthread.h>
#include "base/test.h"

#define TLC_PREFIX TLC_

static pthread_key_t TLC_foobar;

#define TLC_init(name) \
	pthread_key_create(&TLC_ ## name, NULL)

#define TLC_set(name, value) \
	pthread_setspecific(TLC_ ## name, value)

#define TLC_get(name, type) \
	((type*)pthread_getspecific(TLC_ ## name))

typedef struct foo_bar_t
{
	const char* foo;
	const char* bar;
} FooBar;


#define LOG printf
#define SAFE_NULL(ptr) (ptr ? ptr : (char*)0x0)

static void
mylog(const char* message)
{
	FooBar* foobar = TLC_get(foobar, FooBar);

	if (foobar)
		LOG("bar:%p bar:%s - %s\n", SAFE_NULL(foobar->bar), SAFE_NULL(foobar->bar), message);
	else
		LOG("%s\n", message);
}

static void*
pthread_foo_run(void* arg)
{
	FooBar f = { .foo = "hello", .bar = NULL };

	TLC_set(foobar, &f);
	mylog("with foobar");

	return NULL;
}

static void*
pthread_bar_run(void* arg)
{
	mylog("without foobar");
	return NULL;
}

static void
test_Something()
{
	pthread_t foo_thread;
	pthread_t bar_thread;

	TLC_init(foobar);

	PROFILE_BEGIN("100000000 times pthread_setspecific/getspecific")
	pthread_create(&foo_thread, NULL, pthread_foo_run, NULL);
	pthread_create(&bar_thread, NULL, pthread_bar_run, NULL);

	pthread_join(foo_thread, NULL);
	pthread_join(bar_thread, NULL);
	PROFILE_END
//	TEST_ASSERT_EQUAL_PTR(thread, container_of(&thread->pthread, Thread, pthread));
//	printf("ptr pthread:%p ptr:MDC[0]:%p,  %p pthread_size:%lu\n", &thread->pthread, &thread->MDC[0],
//	       (char*)thread + sizeof(pthread_t), sizeof(pthread_t));

//	printf("%s\n", *(thread->MDC[0]));
//	TEST_ASSERT_EQUAL_PTR(thread->MDC[0], thread + sizeof(pthread_t));

//	TEST_ASSERT_EQUAL_PTR("foo", (char*)*thread.MDC);
//	TEST_ASSERT_EQUAL_STRING("foo", TLC_thread_get(&thread, MDC_Foo, const char));
}

int
main()
{
	TEST_BEGIN

	RUN(test_Something);

	TEST_END
}
