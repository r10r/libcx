#include <libcx/base/test.h>
#include "util.h"

static void
test_str_cnt()
{
	TEST_ASSERT_EQUAL_INT(0, str_cnt(NULL, NULL));
	TEST_ASSERT_EQUAL_INT(0, str_cnt(NULL, ","));
	TEST_ASSERT_EQUAL_INT(0, str_cnt("", NULL));
	TEST_ASSERT_EQUAL_INT(0, str_cnt("", ","));
	TEST_ASSERT_EQUAL_INT(3, str_cnt(",,,", ","));
	TEST_ASSERT_EQUAL_INT(2, str_cnt("foo,bar,baz", ","));

	TEST_ASSERT_EQUAL_INT(3, str_cnt("XXbarXXfooXX", "XX"));
	TEST_ASSERT_EQUAL_INT(0, str_cnt("abc", "abcdefg"));
}

static void
test_str_split()
{
	char value[] = "foo,bar,baz";
	const char** arr = NULL;

	unsigned count = str_split(value, ",",  &arr);

	TEST_ASSERT_EQUAL_INT(3, count);
	TEST_ASSERT_EQUAL_STRING("foo", arr[0]);
	TEST_ASSERT_EQUAL_STRING("bar", arr[1]);
	TEST_ASSERT_EQUAL_STRING("baz", arr[2]);
	TEST_ASSERT_NULL(arr[3]);

	cx_free(arr);
}

static void
test_str_split_single()
{
	char value[] = "foo";
	const char** arr = NULL;
	unsigned count = str_split(value, ",",  &arr);

	TEST_ASSERT_EQUAL_INT(1, count);
	TEST_ASSERT_EQUAL_STRING("foo", arr[0]);
	TEST_ASSERT_EQUAL(NULL, arr[1]);

	cx_free(arr);
}

static void
test_str_split_null()
{
	const char** arr = NULL;
	unsigned count = str_split(NULL, ",",  &arr);

	TEST_ASSERT_EQUAL_INT(0, count);
	TEST_ASSERT_NULL(arr);
}

// FIXME edge cases !!!

static void
test_str_arr_merge()
{
	char value1[] = "foo,bar,baz,blub";
	char value2[] = "xxx,blub,foo,yyy";

	const char* expected[] = { "foo", "blub" };

	const char** arr1 = NULL;
	const char** arr2 = NULL;

	unsigned num_arr1 = str_split(value1, ",", &arr1);
	unsigned num_arr2 = str_split(value2, ",", &arr2);

	const char** arr12 = NULL;

	str_arr_intersect(arr1, num_arr1, arr2, num_arr2, &arr12);

	TEST_ASSERT_EQUAL_STRING_ARRAY(expected, arr12, 2);

	cx_free(arr1);
	cx_free(arr2);
	cx_free(arr12);
}

static void
test_str_arr_diff()
{
	char value1[] = "foo,bar,baz,blub";
	char value2[] = "xxx,blub,foo,yyy";

	const char** arr1 = NULL;
	const char** arr2 = NULL;

	unsigned num_arr1 = str_split(value1, ",", &arr1);

	str_split(value2, ",", &arr2);

	const char** arr12 = NULL;

	unsigned remaining = str_arr_diff(arr1, num_arr1, arr2, &arr12);

//	char* elem = NULL;
//	int elem_idx = 0;
//	while ((elem = arr12[elem_idx++]) != NULL)
//	{
//		XFDBG("elem[%d] => %s", elem_idx, elem);
//	}

	const char* expected[] = { "bar", "baz" };
	TEST_ASSERT_EQUAL_INT(2, remaining);
	TEST_ASSERT_EQUAL_STRING_ARRAY(expected, arr12, 2);

	cx_free(arr1);
	cx_free(arr2);
	cx_free(arr12);
}

static void
test_str_arr_diff_empty()
{
	char value1[] = "blubber,Wohnzimmer";
	char value2[] = "blubber,Wohnzimmer";

	const char** arr1 = NULL;
	const char** arr2 = NULL;

	unsigned num_arr1 = str_split(value1, ",", &arr1);

	str_split(value2, ",", &arr2);

	const char** arr12 = NULL;

	unsigned remaining  = str_arr_diff(arr1, num_arr1, arr2, &arr12);

	const char* expected[] = { NULL };
	TEST_ASSERT_EQUAL_INT(0, remaining);
	TEST_ASSERT_EQUAL_STRING_ARRAY(expected, arr12, 1);

	cx_free(arr1);
	cx_free(arr2);
	cx_free(arr12);
}

static void
test_str_join()
{
	const char* vals[] =  { "foo", "bar", "baz", NULL };
	char* s = str_arr_join(vals, "--");

	TEST_ASSERT_EQUAL_STRING("foo--bar--baz", s);

	cx_free(s);
}

int
main()
{
	TEST_BEGIN

	RUN(test_str_cnt);
	RUN(test_str_split);
	RUN(test_str_split_single);
	RUN(test_str_split_null);
	RUN(test_str_arr_merge);
	RUN(test_str_arr_diff);
	RUN(test_str_arr_diff_empty);
	RUN(test_str_join);

	TEST_END
}
