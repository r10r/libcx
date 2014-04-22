#include "base/test.h"
//#include "base/xmalloc.h"
#include "string.h"

static void
test_StringBuffer_append()
{
	StringBuffer* buf = StringBuffer_new(1024);

	TEST_ASSERT_EQUAL_INT(1024, buf->length);
	TEST_ASSERT_EQUAL_INT(0, buf->string->length);
	TEST_ASSERT_EQUAL_INT(1024, StringBuffer_unused(buf));

	TEST_ASSERT_EQUAL_INT(3, StringBuffer_append(buf, 0, "foo", 3));
	TEST_ASSERT_EQUAL_INT(3, buf->string->length);
	TEST_ASSERT_EQUAL_INT(1024 - 3, StringBuffer_unused(buf));

	TEST_ASSERT_EQUAL_INT(3, StringBuffer_append(buf, 3, "bar", 3));
	TEST_ASSERT_EQUAL_INT(6, buf->string->length);
	TEST_ASSERT_EQUAL_INT(1024 - 6, StringBuffer_unused(buf));

	TEST_ASSERT_EQUAL_INT('f', *S_get(buf->string, 0));
	TEST_ASSERT_EQUAL_INT('o', *S_get(buf->string, 1));
	TEST_ASSERT_EQUAL_INT('o', *S_get(buf->string, 2));
	TEST_ASSERT_EQUAL_INT('b', *S_get(buf->string, 3));
	TEST_ASSERT_EQUAL_INT('a', *S_get(buf->string, 4));
	TEST_ASSERT_EQUAL_INT('r', *S_get(buf->string, 5));

	StringBuffer_free(buf);
}

static void
test_StringBuffer_append_grow()
{
	StringBuffer* buf = StringBuffer_new(3);

	TEST_ASSERT_EQUAL_INT(3, buf->length);
	TEST_ASSERT_EQUAL_INT(0, buf->string->length);
	TEST_ASSERT_EQUAL_INT(3, StringBuffer_unused(buf));

	StringBuffer_append(buf, 0, "foo", 3);
	TEST_ASSERT_EQUAL_INT(3, buf->length);
	TEST_ASSERT_EQUAL_INT(3, buf->string->length);
	TEST_ASSERT_EQUAL_INT(0, StringBuffer_unused(buf));

	StringBuffer_append(buf, 3, "foo", 3);
	TEST_ASSERT_EQUAL_INT(6, buf->length);
	TEST_ASSERT_EQUAL_INT(6, buf->string->length);
	TEST_ASSERT_EQUAL_INT(0, StringBuffer_unused(buf));

	StringBuffer_free(buf);
}

static void
test_StringBuffer_append_grow_zero()
{
	StringBuffer* buf = StringBuffer_new(0);

	StringBuffer_append(buf, 0, "foo", 3);
	TEST_ASSERT_EQUAL_INT(3, buf->length);
	TEST_ASSERT_EQUAL_INT(3, buf->string->length);
	TEST_ASSERT_EQUAL_INT(0, StringBuffer_unused(buf));
	;

	StringBuffer_free(buf);
}

static void
test_StringBuffer_scat()
{
	StringBuffer* buf = StringBuffer_new(0);
	String* s = S_dup("foobar");

	StringBuffer_scat(buf, s);
	TEST_ASSERT_EQUAL_INT(6, buf->length);
	TEST_ASSERT_EQUAL_INT(6, buf->string->length);
	TEST_ASSERT_EQUAL_INT(0, StringBuffer_unused(buf));

	TEST_ASSERT_EQUAL(0, S_comp(s, buf->string));

	S_free(s);
	StringBuffer_free(buf);
}

static void
test_StringBuffer_fcat()
{
	StringBuffer* buf = StringBuffer_new(0);

	char template[] = "/tmp/temp.XXXXXX";
	int fd = mkstemp(template);
	FILE* tmpfile = fdopen(fd, "w+");

	fwrite("foo", 1, 4, tmpfile);


	// buffer is extended to maximum read size each time before reading
	rewind(tmpfile);
	TEST_ASSERT_EQUAL_INT(4, StringBuffer_fncat(buf, tmpfile, 32));
	TEST_ASSERT_EQUAL_INT(32, buf->length);
	TEST_ASSERT_EQUAL_INT(4, buf->string->length);
	TEST_ASSERT_EQUAL_INT(32 - 4, StringBuffer_unused(buf));

	rewind(tmpfile);
	TEST_ASSERT_EQUAL_INT(4, StringBuffer_fncat(buf, tmpfile, 32));
	TEST_ASSERT_EQUAL_INT(32 + 4, buf->length);

	rewind(tmpfile);
	TEST_ASSERT_EQUAL_INT(4, StringBuffer_fncat(buf, tmpfile, 32));
	TEST_ASSERT_EQUAL_INT(32 + 8, buf->length);

	TEST_ASSERT_EQUAL_INT(0, StringBuffer_fncat(buf, tmpfile, 32));
	TEST_ASSERT_EQUAL_INT(32 + 12, buf->length);

	// additional reads do not change anything
	TEST_ASSERT_EQUAL_INT(0, StringBuffer_fncat(buf, tmpfile, 32));
	TEST_ASSERT_EQUAL_INT(0, StringBuffer_fncat(buf, tmpfile, 32));
	TEST_ASSERT_EQUAL_INT(0, StringBuffer_fncat(buf, tmpfile, 32));

	TEST_ASSERT_EQUAL_INT(4 + 4 + 4, buf->string->length);
	TEST_ASSERT_EQUAL_INT(32, StringBuffer_unused(buf));

	StringBuffer_free(buf);
	fclose(tmpfile);
	unlink(template);
}

static void
test_StringBuffer_append_grow_offset()
{
	StringBuffer* buf = StringBuffer_new(3);

	TEST_ASSERT_EQUAL_INT(3, buf->length);
	TEST_ASSERT_EQUAL_INT(0, buf->string->length);
	TEST_ASSERT_EQUAL_INT(3, StringBuffer_unused(buf));

	StringBuffer_append(buf, 0, "foo", 3);
	TEST_ASSERT_EQUAL_INT(3, buf->length);
	TEST_ASSERT_EQUAL_INT(3, buf->string->length);
	TEST_ASSERT_EQUAL_INT(0, StringBuffer_unused(buf));

	StringBuffer_append(buf, 2, "foo", 3);
	TEST_ASSERT_EQUAL_INT(5, buf->length);
	TEST_ASSERT_EQUAL_INT(5, buf->string->length);
	TEST_ASSERT_EQUAL_INT(0, StringBuffer_unused(buf));

	StringBuffer_free(buf);
}

static void
test_limits()
{
	String* s = String_init(NULL, STRING_MAX_LENGTH + 1);

	TEST_ASSERT_NULL(s);

	StringBuffer* b = StringBuffer_new(STRING_MAX_LENGTH + 1);
	TEST_ASSERT_NULL(b);

	b = StringBuffer_new(STRING_MAX_LENGTH);
	TEST_ASSERT_NOT_NULL(b);

	unsigned int i;
	unsigned int iterations = 500;
	const char* pattern = "foobar";
	size_t pattern_length = strlen(pattern) - 1;

	for (i = 0; i < iterations; i++)
		StringBuffer_ncat(b, pattern, pattern_length);

	TEST_ASSERT_EQUAL(pattern_length * iterations, b->string->length);

	S_free(s);
	StringBuffer_free(b);
}

static void
test_StringBuffer_shift()
{
	StringBuffer* buf = StringBuffer_new(12);

	StringBuffer_catn(buf, "foobar");
	TEST_ASSERT_EQUAL_INT(7, buf->string->length);
	TEST_ASSERT_EQUAL_INT(5, StringBuffer_unused(buf));

	StringBuffer_shift(buf, 3);
	TEST_ASSERT_EQUAL_STRING("bar", buf->string->value);
	TEST_ASSERT_EQUAL_INT(8, StringBuffer_unused(buf));

	StringBuffer_free(buf);
}

static void
test_StringBuffer_clear()
{
	StringBuffer* buf = StringBuffer_new(12);

	StringBuffer_catn(buf, "foo");
	TEST_ASSERT_EQUAL_INT(4, StringBuffer_used(buf));
	TEST_ASSERT_EQUAL_INT(8, StringBuffer_unused(buf));

	StringBuffer_clear(buf);
	TEST_ASSERT_EQUAL_INT(0, StringBuffer_used(buf));
	TEST_ASSERT_EQUAL_INT(12, StringBuffer_unused(buf));

	StringBuffer_catn(buf, "bar");
	TEST_ASSERT_EQUAL_STRING("bar", StringBuffer_value(buf));
	TEST_ASSERT_EQUAL_INT(8, StringBuffer_unused(buf));

	StringBuffer_free(buf);
}

static void
test_StringBuffer_printf()
{
	StringBuffer* buf = StringBuffer_new(6);

	StringBuffer_printf(buf, "%s %s", "hello", "world");

	TEST_ASSERT_EQUAL_STRING("hello world", StringBuffer_value(buf));
	TEST_ASSERT_EQUAL_INT(12, StringBuffer_length(buf));
	TEST_ASSERT_EQUAL_INT(0, StringBuffer_unused(buf));

	StringBuffer_free(buf);
}

int
main()
{
	TEST_BEGIN

	RUN(test_StringBuffer_append);
	RUN(test_StringBuffer_append_grow);
	RUN(test_StringBuffer_append_grow_zero);
	RUN(test_StringBuffer_scat);
	RUN(test_StringBuffer_fcat);
	RUN(test_StringBuffer_append_grow_offset);
	RUN(test_limits);
	RUN(test_StringBuffer_shift);
	RUN(test_StringBuffer_clear);
	RUN(test_StringBuffer_printf);
	TEST_END
}
