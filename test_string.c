#include "libcx-base/test.h"
//#include "libcx-base/xmalloc.h"
#include "string.h"

NOSETUP

static void
test_String_dup()
{
	String *s = S_dup(NULL);

	TEST_ASSERT_NULL(s);

	s = S_dup("bar");
	TEST_ASSERT_EQUAL_INT(s->length, 3);

	TEST_ASSERT_EQUAL_INT('b', S_at(s, 0));
	TEST_ASSERT_EQUAL_INT('a', S_at(s, 1));
	TEST_ASSERT_EQUAL_INT('r', S_at(s, 2));

	TEST_ASSERT_EQUAL_INT('b', S_at(s, -3));
	TEST_ASSERT_EQUAL_INT('a', S_at(s, -2));
	TEST_ASSERT_EQUAL_INT('r', S_at(s, -1));

	free(s);
}

static void
test_S_comp()
{
	String *a = S_dup("bar");
	String *b = S_dup("foo");
	String *c = S_dup("foobar");

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
test_String_Buffer_ncopy()
{
	StringBuffer *buf = StringBuffer_new(1024);

	TEST_ASSERT_EQUAL_INT(1024, buf->length);
	TEST_ASSERT_EQUAL_INT(0, buf->string->length);
	TEST_ASSERT_EQUAL_INT(1024, SBuf_unused(buf));

	TEST_ASSERT_EQUAL_INT(3, StringBuffer_ncopy(buf, 0, "foo", 3));
	TEST_ASSERT_EQUAL_INT(3, buf->string->length);
	TEST_ASSERT_EQUAL_INT(1024 - 3, SBuf_unused(buf));

	TEST_ASSERT_EQUAL_INT(3, StringBuffer_ncopy(buf, 3, "bar", 3));
	TEST_ASSERT_EQUAL_INT(6, buf->string->length);
	TEST_ASSERT_EQUAL_INT(1024 - 6, SBuf_unused(buf));

	TEST_ASSERT_EQUAL_INT('f', S_at(buf->string, 0));
	TEST_ASSERT_EQUAL_INT('o', S_at(buf->string, 1));
	TEST_ASSERT_EQUAL_INT('o', S_at(buf->string, 2));
	TEST_ASSERT_EQUAL_INT('b', S_at(buf->string, 3));
	TEST_ASSERT_EQUAL_INT('a', S_at(buf->string, 4));
	TEST_ASSERT_EQUAL_INT('r', S_at(buf->string, 5));


	SBuf_free(buf);
}

static void
test_String_Buffer_ncopy_grow()
{
	StringBuffer *buf = StringBuffer_new(3);

	TEST_ASSERT_EQUAL_INT(3, buf->length);
	TEST_ASSERT_EQUAL_INT(0, buf->string->length);
	TEST_ASSERT_EQUAL_INT(3, SBuf_unused(buf));

	StringBuffer_ncopy(buf, 0, "foo", 3);
	TEST_ASSERT_EQUAL_INT(3, buf->length);
	TEST_ASSERT_EQUAL_INT(3, buf->string->length);
	TEST_ASSERT_EQUAL_INT(0, SBuf_unused(buf));

	StringBuffer_ncopy(buf, 3, "foo", 3);
	TEST_ASSERT_EQUAL_INT(6, buf->length);
	TEST_ASSERT_EQUAL_INT(6, buf->string->length);
	TEST_ASSERT_EQUAL_INT(0, SBuf_unused(buf));

	SBuf_free(buf);
}

static void
test_String_Buffer_ncopy_grow_zero()
{
	StringBuffer *buf = StringBuffer_new(0);

	StringBuffer_ncopy(buf, 0, "foo", 3);
	TEST_ASSERT_EQUAL_INT(3, buf->length);
	TEST_ASSERT_EQUAL_INT(3, buf->string->length);
	TEST_ASSERT_EQUAL_INT(0, SBuf_unused(buf));;

	SBuf_free(buf);
}

static void
test_String_Buffer_cat()
{
	StringBuffer *buf = StringBuffer_new(0);
	String *s = S_dup("foobar");

	StringBuffer_cat(buf, s);
	TEST_ASSERT_EQUAL_INT(6, buf->length);
	TEST_ASSERT_EQUAL_INT(6, buf->string->length);
	TEST_ASSERT_EQUAL_INT(0, SBuf_unused(buf));;

	TEST_ASSERT_EQUAL(0, S_comp(s, buf->string));

	S_free(s);
	SBuf_free(buf);
}

//static void
//test_String_Buffer_ncat()
//{
//	StringBuffer *buf = StringBuffer_new(0);
//	String *s = S_dup("foobar");
//
//	StringBuffer_cat(buf, s);
//	TEST_ASSERT_EQUAL_INT(6, buf->length);
//	TEST_ASSERT_EQUAL_INT(6, buf->string->length);
//	TEST_ASSERT_EQUAL_INT(0, SBuf_unused(buf));;
//
//	TEST_ASSERT_EQUAL(0, S_comp(s, buf->string));
//
//	S_free(s);
//	SBuf_free(buf);
//}

int main()
{
	TEST_BEGIN

	RUN(test_S_comp);
	RUN(test_String_dup);
	RUN(test_String_Buffer_ncopy);
	RUN(test_String_Buffer_ncopy_grow);
	RUN(test_String_Buffer_ncopy_grow_zero);
	RUN(test_String_Buffer_cat);
//	RUN(test_String_Buffer_ncat);

	TEST_END
}
