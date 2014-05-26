#include "../base/test.h"
#include "pair.h"

static void
test_StringPair_new()
{
	StringPair* pair = StringPair_new("foo", "bar");

	StringPair_free(pair);
}

static void
test_StringPair_init()
{
	String* a = S_dup("aaa");
	String* b = S_dup("bbb");
	StringPair* ab  = StringPair_init(a, b);

	TEST_ASSERT_EQUAL_INT(0, S_comp(ab->key, a));
	TEST_ASSERT_EQUAL_INT(0, S_comp(ab->value, b));

	StringPair_free(ab);
}

int
main()
{
	TEST_BEGIN

	RUN(test_StringPair_new);
	RUN(test_StringPair_init);

	TEST_END
}
