#include <libcx/base/test.h>

#define _CX_LIST_CONCURRENT
#include "concurrent_list.h"

static const char* const foo = "foo";
static const char* const bar = "bar";
static const char* const baz = "baz";

static void
test_ListConcurrent()
{
	ConcurrentList* list = cx_alloc(sizeof(ConcurrentList));

	ConcurrentList_init(list);
	list->list.f_node_data_free = NULL;

	ConcurrentList_append(list, foo);
	ConcurrentList_append(list, bar);
	ConcurrentList_append(list, baz);

	const char* data;

	/* get */

	ConcurrentList_get(list, 0, data);
	TEST_ASSERT_EQUAL_STRING(foo, data);

	ConcurrentList_get(list, 1, data);
	TEST_ASSERT_EQUAL_STRING(bar, data);

	ConcurrentList_get(list, 2, data);
	TEST_ASSERT_EQUAL_STRING(baz, data);

	/* pop */
	ConcurrentList_pop(list, data);
	TEST_ASSERT_EQUAL_STRING(baz, data);

	ConcurrentList_pop(list, data);
	TEST_ASSERT_EQUAL_STRING(bar, data);

	ConcurrentList_pop(list, data);
	TEST_ASSERT_EQUAL_STRING(foo, data);

	TEST_ASSERT_EQUAL_INT(0, list->list.length);

	ConcurrentList_free(list);
}

static void
test_ListConcurrent2()
{
	ConcurrentList* list = cx_alloc(sizeof(ConcurrentList));

	ConcurrentList_init(list);
	list->list.f_node_data_free = NULL;

	ConcurrentList_append(list, foo);
	ConcurrentList_append(list, bar);
	ConcurrentList_append(list, baz);

	ConcurrentList_delete(list, 1);

	const char* data;

	ConcurrentList_shift(list, data);
	TEST_ASSERT_EQUAL_STRING(foo, data);

//	ConcurrentList_shift(list, data);
//	TEST_ASSERT_EQUAL_STRING(bar, data);

	ConcurrentList_shift(list, data);
	TEST_ASSERT_EQUAL_STRING(baz, data);

	ConcurrentList_free(list);
}

static void
test_ListConcurrent3()
{
	ConcurrentList* list = cx_alloc(sizeof(ConcurrentList));

	ConcurrentList_init(list);
	list->list.f_node_data_free = NULL;

	ConcurrentList_append(list, foo);
	ConcurrentList_append(list, bar);
	ConcurrentList_append(list, baz);

	ConcurrentList_remove(list, bar);
	ConcurrentList_unshift(list, bar);

	const char* data;

	ConcurrentList_shift(list, data);
	TEST_ASSERT_EQUAL_STRING(bar, data);

	ConcurrentList_shift(list, data);
	TEST_ASSERT_EQUAL_STRING(foo, data);

	ConcurrentList_shift(list, data);
	TEST_ASSERT_EQUAL_STRING(baz, data);

	ConcurrentList_free(list);
}

int
main()
{
	TEST_BEGIN
	RUN(test_ListConcurrent);
	RUN(test_ListConcurrent2);
	RUN(test_ListConcurrent3);
	TEST_END
}
