#include "libcx-base/test.h"
#include "libcx-base/xmalloc.h"
#include "string.h"

NOSETUP

void test_String_new()
{
	String s = String_new("foo");

	printf("%s\n", s);
	TEST_ASSERT_EQUAL_INT(3, String_space(s));
	String_free(s);
}

void test_String_append()
{
	String foo = String_new("foo");
	String bar = String_new("bar");
	String foobar = String_append(foo, bar);

	printf("%s\n", foobar);
	TEST_ASSERT_EQUAL_INT(6, String_space(foobar));
	TEST_ASSERT_EQUAL_STRING("foobar", foobar);
	String_free(foobar);
	String_free(bar);
}

void test_StringPair_new()
{
	Pair *foobar = StringPair_new("foo", "bar");

	TEST_ASSERT_EQUAL_STRING("foo", foobar->key);
	TEST_ASSERT_EQUAL_STRING("bar", foobar->value);
	StringPair_free(foobar);
}

int main()
{
	TEST_BEGIN

	RUN(test_String_new);
	RUN(test_String_append);
	RUN(test_StringPair_new);

	TEST_END
}
