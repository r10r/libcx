#include "base/test.h"
//#include "base/xmalloc.h"
#include "string.h"

static void
test_S_dup()
{
	String* s = S_dup("bar");

	TEST_ASSERT_EQUAL_STRING("bar", s->value);
	TEST_ASSERT_EQUAL_INT(3, s->length);

	TEST_ASSERT_EQUAL_INT('b', *S_get(s, 0));
	TEST_ASSERT_EQUAL_INT('a', *S_get(s, 1));
	TEST_ASSERT_EQUAL_INT('r', *S_get(s, 2));

	TEST_ASSERT_EQUAL_INT('\0', *S_get(s, 3)); // the hidden \0 terminator

	TEST_ASSERT_EQUAL_INT('b', *S_get(s, -3));
	TEST_ASSERT_EQUAL_INT('a', *S_get(s, -2));
	TEST_ASSERT_EQUAL_INT('r', *S_get(s, -1));

	TEST_ASSERT_EQUAL_INT('r', *S_last(s));

	free(s);
}

static void
test_S_sget()
{
	String* s = S_dup("bar");

	TEST_ASSERT_EQUAL_INT('b', *S_sget(s, 0));
	TEST_ASSERT_EQUAL_INT('a', *S_sget(s, 1));
	TEST_ASSERT_EQUAL_INT('r', *S_sget(s, 2));

	TEST_ASSERT_EQUAL_INT('b', *S_sget(s, -3));
	TEST_ASSERT_EQUAL_INT('a', *S_sget(s, -2));
	TEST_ASSERT_EQUAL_INT('r', *S_sget(s, -1));

	TEST_ASSERT_NULL(S_sget(s, 4));
	TEST_ASSERT_NULL(S_sget(s, -4));

	free(s);
}

static void
test_empty_string()
{
	String* szero = String_init(NULL, 0);
	String* sempty = String_init(NULL, 1024);

	TEST_ASSERT_EQUAL_INT(0, szero->length);
	TEST_ASSERT_EQUAL_INT(0, sempty->length);

	TEST_ASSERT_EQUAL_STRING("", szero);
	TEST_ASSERT_EQUAL_STRING("", sempty);


	free(szero);
	free(sempty);
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
	String* s = S_dup("foobar");
	char dest[7];

	S_copy(s, dest);
	TEST_ASSERT_EQUAL_STRING("foobar", dest);
	free(s);
}

static void
test_String_shift()
{
	String* s = S_dup("foobar");

	TEST_ASSERT_EQUAL_INT(6, s->length);
	TEST_ASSERT_EQUAL_INT(1, String_shift(s, 3));
	TEST_ASSERT_EQUAL_STRING("bar", s->value);
	TEST_ASSERT_EQUAL_INT(3, s->length);

	/* shifting nothing should return 0 */
	TEST_ASSERT_EQUAL_INT(0, String_shift(s, 0));
	TEST_ASSERT_EQUAL_INT(3, s->length);

	/* shifting to much should do nothing and return -1 */
	TEST_ASSERT_EQUAL_INT(-1, String_shift(s, 5));
	TEST_ASSERT_EQUAL_INT(3, s->length);

	/* shifting the remaining content*/
	TEST_ASSERT_EQUAL_INT(1, String_shift(s, 3));
	TEST_ASSERT_EQUAL_INT(0, s->length);

	free(s);
}

static void
test_S_clear()
{
	String* s = S_dup("foobar");

	TEST_ASSERT_EQUAL_INT(6, s->length);

	TEST_ASSERT_EQUAL_INT(1, S_clear(s));
	TEST_ASSERT_EQUAL_STRING("", s->value);
	TEST_ASSERT_EQUAL_INT(0, S_clear(s));

	free(s);
}

int
main()
{
	TEST_BEGIN

	RUN(test_S_dup);
	RUN(test_S_sget);
	RUN(test_empty_string);
	RUN(test_S_comp);
	RUN(test_S_write);
	RUN(test_S_copy);
	RUN(test_String_shift);
	RUN(test_S_clear);
	TEST_END
}
