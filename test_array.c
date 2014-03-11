#include "libcx-base/test.h"
#include "array.h"

NOSETUP

void test_Array_new_empty()
{
	Array a = Array_new(0);

	TEST_ASSERT_EQUAL(0, Array_length(a));
	TEST_ASSERT_EQUAL(0, Array_unused(a));
	TEST_ASSERT_EQUAL(0, Array_next(a));
	Array_free(a);
}

void test_Array_new()
{
	Array a = Array_new(2);

	TEST_ASSERT_EQUAL(2, Array_length(a));
	TEST_ASSERT_EQUAL(2, Array_unused(a));
	TEST_ASSERT_EQUAL(0, Array_next(a));

	Array_free(a);
}

void test_Array_grow()
{
	Array a = Array_new(0);

	a = Array_grow(a, 10);
	TEST_ASSERT_EQUAL(10, Array_length(a));
	TEST_ASSERT_EQUAL(10, Array_unused(a));
	TEST_ASSERT_EQUAL(0, Array_next(a));

	Array_free(a);
}

void test_Array_push()
{
	char *first = "foo";
	char *second = "bar";
	char *third = "third";

	Array a = Array_new(2);

	TEST_ASSERT_EQUAL(0, Array_next(a));
	TEST_ASSERT_EQUAL(2, Array_unused(a));

	a = Array_push(a, first);
	TEST_ASSERT_EQUAL_PTR(first, Array_last(a));
	// test access by index
	TEST_ASSERT_EQUAL_STRING(first, a[0]);
	TEST_ASSERT_EQUAL(1, Array_next(a));
	TEST_ASSERT_EQUAL(1, Array_unused(a));

	a = Array_push(a, second);
	TEST_ASSERT_EQUAL_PTR(second, Array_last(a));
	// test access by index
	TEST_ASSERT_EQUAL_STRING(second, a[1]);
	TEST_ASSERT_EQUAL(2, Array_next(a));
	TEST_ASSERT_EQUAL(0, Array_unused(a));

	a = Array_push(a, third);
	TEST_ASSERT_EQUAL_PTR(third, Array_last(a));
	TEST_ASSERT_EQUAL_STRING(third, a[2]);
	// test access by index
	TEST_ASSERT_EQUAL(3, Array_next(a));
	TEST_ASSERT_EQUAL(2 + ARRAY_GROW_LENGTH - 3, Array_unused(a));

	Array_free(a);
}

void test_Array_pop()
{
	char *first = "foo";
	char *second = "bar";
	char *third = "third";

	Array a = Array_new(0);

	a = Array_append(a, first);
	a = Array_append(a, second);
	a = Array_append(a, third);
	TEST_ASSERT_EQUAL(3, Array_next(a));

	TEST_ASSERT_EQUAL_STRING(third, Array_pop(a));
	TEST_ASSERT_EQUAL(2, Array_next(a));

	TEST_ASSERT_EQUAL_STRING(second, Array_pop(a));
	TEST_ASSERT_EQUAL(1, Array_next(a));

	TEST_ASSERT_EQUAL_STRING(first, Array_pop(a));
	TEST_ASSERT_EQUAL(0, Array_next(a));

	Array_free(a);
}

static int
match_element(void *data, void *key)
{
	return strcmp((char*)data, (char*)key);
}

void test_Array_match()
{
	char *first = "foo";
	char *second = "bar";
	char *third = "third";

	Array a = Array_new(0);

	a = Array_append(a, first);
	a = Array_append(a, second);
	a = Array_append(a, third);

	TEST_ASSERT_EQUAL(0, Array_match(a, first, match_element));
	TEST_ASSERT_EQUAL(1, Array_match(a, second, match_element));
	TEST_ASSERT_EQUAL(2, Array_match(a, third, match_element));

	Array_free(a);
}

static void
element_iterator(int index, void *data)
{
	if (index == 0)
		TEST_ASSERT_EQUAL_STRING("foo", (char*)data);
	if (index == 1)
		TEST_ASSERT_EQUAL_STRING("bar", (char*)data);
	if (index == 2)
		TEST_ASSERT_EQUAL_STRING("third", (char*)data);
}

void test_Array_each()
{
	char *first = "foo";
	char *second = "bar";
	char *third = "third";

	Array a = Array_new(0);

	a = Array_append(a, first);
	a = Array_append(a, second);
	a = Array_append(a, third);

	Array_each(a, element_iterator);

	Array_free(a);
}

int main()
{
	TEST_BEGIN

	RUN(test_Array_new_empty);
	RUN(test_Array_new);
	RUN(test_Array_grow);
	RUN(test_Array_push);
	RUN(test_Array_pop);
	RUN(test_Array_match);
	RUN(test_Array_each);

	TEST_END
}
