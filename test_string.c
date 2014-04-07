#include "libcx-base/test.h"
//#include "libcx-base/xmalloc.h"
#include "string.h"

static void
test_String_dup()
{
	String* s = S_dup("bar");

	TEST_ASSERT_EQUAL_INT(s->length, 3);

	printf("%p %p %c\n", s->value, &s->value[0], *s->value);

	TEST_ASSERT_EQUAL_INT('b', *S_get(s, 0));
	TEST_ASSERT_EQUAL_INT('a', *S_get(s, 1));
	TEST_ASSERT_EQUAL_INT('r', *S_get(s, 2));

	TEST_ASSERT_EQUAL_INT('b', *S_get(s, -3));
	TEST_ASSERT_EQUAL_INT('a', *S_get(s, -2));
	TEST_ASSERT_EQUAL_INT('r', *S_get(s, -1));

	free(s);
}

static void
test_S_comp()
{
	String* a = S_dup("bar");
	String* b = S_dup("foo");
	String* c = S_dup("foobar");

	TEST_ASSERT_TRUE(S_comp(a, a) == 0);
	TEST_ASSERT_TRUE(S_comp(a, b) < 0);
	TEST_ASSERT_TRUE(S_comp(b, a) > 0);
	TEST_ASSERT_TRUE(S_comp(a, c) < 0);
	TEST_ASSERT_TRUE(S_comp(c, a) > 0);

	S_free(a);
	S_free(b);
	S_free(c);
}

static void
test_S_dupn()
{
	String* s = S_dupn("foobar");

	TEST_ASSERT_EQUAL_STRING("foobar", s->value);
	S_free(s);
}

static void
test_S_write()
{
	String* s = S_dup("foobar");

	TEST_ASSERT_EQUAL_INT(6, S_fwrite(s, stdout));
	TEST_ASSERT_EQUAL_INT(7, S_fputs(s, stdout));

	free(s);
}

static void
test_S_copy()
{
	String* s = S_dupn("foobar");
	char dest[7];

	S_copy(s, dest);
	TEST_ASSERT_EQUAL_STRING("foobar", dest);
	free(s);
}

static void
test_String_shift()
{
	String* s = S_dupn("foobar");

	TEST_ASSERT_EQUAL_INT(7, s->length);
	TEST_ASSERT_EQUAL_INT(1, String_shift(s, 3));
	TEST_ASSERT_EQUAL_STRING("bar", s->value);
	TEST_ASSERT_EQUAL_INT(4, s->length);

	/* shifting nothing should return 0 */
	TEST_ASSERT_EQUAL_INT(0, String_shift(s, 0));
	TEST_ASSERT_EQUAL_INT(4, s->length);

	/* shifting to much should do nothing and return -1 */
	TEST_ASSERT_EQUAL_INT(-1, String_shift(s, 5));
	TEST_ASSERT_EQUAL_INT(4, s->length);

	/* shifting the remaining content*/
	TEST_ASSERT_EQUAL_INT(1, String_shift(s, 4));
	TEST_ASSERT_EQUAL_INT(0, s->length);

	free(s);
}

int
main()
{
	TEST_BEGIN

	RUN(test_S_comp);
	RUN(test_String_dup);
	RUN(test_S_dupn);
	RUN(test_S_write);
	RUN(test_S_copy);
	RUN(test_String_shift);
	TEST_END
}
