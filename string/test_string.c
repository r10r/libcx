#include <libcx/base/test.h>

#include "string.h"

static void
test_S_dup_null()
{
	TEST_ASSERT_NULL(S_dup(NULL));
}

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

	S_free(s);
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

	S_free(s);
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


	S_free(szero);
	S_free(sempty);
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

	S_free(s);
}

static void
test_S_copy()
{
	String* s = S_dup("foobar");
	char dest[7];

	S_copy(s, dest);
	TEST_ASSERT_EQUAL_STRING("foobar", dest);
	S_free(s);
}

int
main()
{
	TEST_BEGIN

	RUN(test_S_dup_null);
	RUN(test_S_dup);
	RUN(test_S_sget);
	RUN(test_empty_string);
	RUN(test_S_comp);
	RUN(test_S_write);
	RUN(test_S_copy);
	TEST_END
}
