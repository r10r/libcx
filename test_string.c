#include "libcx-base/test.h"
#include "libcx-base/xmalloc.h"
#include "string.h"

NOSETUP

void test_String_new()
{
	String s = String_new("foo");
	printf("%s\n", s);
	TEST_ASSERT_EQUAL_INT(3, String_length(s));
	String_free(s);
}

void test_String_append()
{
	String foo = String_new("foo");
	String bar = String_new("bar");
	String foobar = String_append(foo, bar);
	printf("%s\n", foobar);
	TEST_ASSERT_EQUAL_INT(6, String_length(foobar));
	String_free(foobar);
	String_free(bar);
}

//void test_String_append()
//{
//	String *f = String_ndup("foo");
//	String *b = String_ndup("bar");
//	String *foobar = String_concat(f, b);
//	String_puts(foobar, stdout);
//	String_free(foobar);
//}

int main()
{
	TEST_BEGIN

	RUN(test_String_new);
	RUN(test_String_append);

	TEST_END
}
