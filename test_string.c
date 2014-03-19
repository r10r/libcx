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

	TEST_ASSERT_EQUAL_INT('b', S_get(s, 0));
	TEST_ASSERT_EQUAL_INT('a', S_get(s, 1));
	TEST_ASSERT_EQUAL_INT('r', S_get(s, 2));

	TEST_ASSERT_EQUAL_INT('b', S_get(s, -3));
	TEST_ASSERT_EQUAL_INT('a', S_get(s, -2));
	TEST_ASSERT_EQUAL_INT('r', S_get(s, -1));

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

	TEST_ASSERT_EQUAL_INT('f', S_get(buf->string, 0));
	TEST_ASSERT_EQUAL_INT('o', S_get(buf->string, 1));
	TEST_ASSERT_EQUAL_INT('o', S_get(buf->string, 2));
	TEST_ASSERT_EQUAL_INT('b', S_get(buf->string, 3));
	TEST_ASSERT_EQUAL_INT('a', S_get(buf->string, 4));
	TEST_ASSERT_EQUAL_INT('r', S_get(buf->string, 5));

	StringBuffer_free(buf);
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

	StringBuffer_free(buf);
}

static void
test_String_Buffer_ncopy_grow_zero()
{
	StringBuffer *buf = StringBuffer_new(0);

	StringBuffer_ncopy(buf, 0, "foo", 3);
	TEST_ASSERT_EQUAL_INT(3, buf->length);
	TEST_ASSERT_EQUAL_INT(3, buf->string->length);
	TEST_ASSERT_EQUAL_INT(0, SBuf_unused(buf));;

	StringBuffer_free(buf);
}

static void
test_String_Buffer_cat()
{
	StringBuffer *buf = StringBuffer_new(0);
	String *s = S_dup("foobar");

	StringBuffer_cat(buf, s);
	TEST_ASSERT_EQUAL_INT(6, buf->length);
	TEST_ASSERT_EQUAL_INT(6, buf->string->length);
	TEST_ASSERT_EQUAL_INT(0, SBuf_unused(buf));

	TEST_ASSERT_EQUAL(0, S_comp(s, buf->string));

	S_free(s);
	StringBuffer_free(buf);
}

static void
test_String_Buffer_fread_append()
{
	StringBuffer *buf = StringBuffer_new(0);

	char template[] = "/tmp/temp.XXXXXX";
	int fd = mkstemp(template);
	FILE *tmpfile = fdopen(fd, "w+");

	fwrite("foo", 1, 4, tmpfile);


	// buffer is extended to maximum read size each time before reading
	rewind(tmpfile);
	TEST_ASSERT_EQUAL_INT(4, StringBuffer_fread_append(buf, tmpfile, 32));
	TEST_ASSERT_EQUAL_INT(32, buf->length);
	TEST_ASSERT_EQUAL_INT(4, buf->string->length);
	TEST_ASSERT_EQUAL_INT(32 - 4, SBuf_unused(buf));

	rewind(tmpfile);
	TEST_ASSERT_EQUAL_INT(4, StringBuffer_fread_append(buf, tmpfile, 32));
	TEST_ASSERT_EQUAL_INT(32 + 4, buf->length);

	rewind(tmpfile);
	TEST_ASSERT_EQUAL_INT(4, StringBuffer_fread_append(buf, tmpfile, 32));
	TEST_ASSERT_EQUAL_INT(32 + 8, buf->length);

	TEST_ASSERT_EQUAL_INT(0, StringBuffer_fread_append(buf, tmpfile, 32));
	TEST_ASSERT_EQUAL_INT(32 + 12, buf->length);

	// additional reads do not change anything
	TEST_ASSERT_EQUAL_INT(0, StringBuffer_fread_append(buf, tmpfile, 32));
	TEST_ASSERT_EQUAL_INT(0, StringBuffer_fread_append(buf, tmpfile, 32));
	TEST_ASSERT_EQUAL_INT(0, StringBuffer_fread_append(buf, tmpfile, 32));

	TEST_ASSERT_EQUAL_INT(4 + 4 + 4, buf->string->length);
	TEST_ASSERT_EQUAL_INT(32, SBuf_unused(buf));

	StringBuffer_free(buf);
	fclose(tmpfile);
	unlink(template);
}

// easy method to compare the string ?
static void
test_String_Buffer_ncopy_grow_index()
{
	StringBuffer *buf = StringBuffer_new(3);

	TEST_ASSERT_EQUAL_INT(3, buf->length);
	TEST_ASSERT_EQUAL_INT(0, buf->string->length);
	TEST_ASSERT_EQUAL_INT(3, SBuf_unused(buf));

	StringBuffer_ncopy(buf, 0, "foo", 3);
	TEST_ASSERT_EQUAL_INT(3, buf->length);
	TEST_ASSERT_EQUAL_INT(3, buf->string->length);
	TEST_ASSERT_EQUAL_INT(0, SBuf_unused(buf));

	StringBuffer_ncopy(buf, 2, "foo", 3);
	TEST_ASSERT_EQUAL_INT(5, buf->length);
	TEST_ASSERT_EQUAL_INT(5, buf->string->length);
	TEST_ASSERT_EQUAL_INT(0, SBuf_unused(buf));

	StringBuffer_free(buf);
}

static void
test_limits()
{
	String *s = String_init(NULL, STRING_MAX_LENGTH + 1);

	TEST_ASSERT_NULL(s);

	StringBuffer *b = StringBuffer_new(STRING_MAX_LENGTH + 1);
	TEST_ASSERT_NULL(b);

	b = StringBuffer_new(STRING_MAX_LENGTH);
	TEST_ASSERT_NOT_NULL(b);

	unsigned int i;
	unsigned int iterations = 500;
	const char *pattern = "foobar";
	size_t pattern_length = strlen(pattern) - 1;

	for (i = 0; i < iterations; i++)
		StringBuffer_nappend(b, pattern, pattern_length);

	TEST_ASSERT_EQUAL(pattern_length * iterations, b->string->length);

	S_free(s);
	StringBuffer_free(b);
}

int main()
{
	TEST_BEGIN

	RUN(test_S_comp);
	RUN(test_String_dup);
	RUN(test_String_Buffer_ncopy);
	RUN(test_String_Buffer_ncopy_grow);
	RUN(test_String_Buffer_ncopy_grow_zero);
	RUN(test_String_Buffer_cat);
	RUN(test_String_Buffer_fread_append);
	RUN(test_String_Buffer_ncopy_grow_index);
	RUN(test_limits);

	TEST_END
}
