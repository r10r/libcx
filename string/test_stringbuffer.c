#include "base/test.h"
//#include "base/xmalloc.h"
#include "string_buffer.h"

#define TEST_ASSERT_BUFFER(buf, s, blen, used, unused) \
	TEST_ASSERT_EQUAL_STRING(s, StringBuffer_value(buf)); \
	TEST_ASSERT_EQUAL_INT(blen, StringBuffer_length(buf)); \
	TEST_ASSERT_EQUAL_INT(used, StringBuffer_used(buf)); \
	TEST_ASSERT_EQUAL_INT(unused, StringBuffer_unused(buf));

static void
test_StringBuffer_cat()
{
	StringBuffer* buf = StringBuffer_new(1024);

	TEST_ASSERT_BUFFER(buf, "", 1024, 0, 1024);

	TEST_ASSERT_EQUAL_INT(4, StringBuffer_cat(buf, "foo"));
	TEST_ASSERT_BUFFER(buf, "foo", 1024, 4, 1024 - 4);

	TEST_ASSERT_EQUAL_INT(4, StringBuffer_cat(buf, "bar"));
	TEST_ASSERT_BUFFER(buf, "foobar", 1024, 7, 1024 - 7);

	TEST_ASSERT_EQUAL_INT('f', *S_get(buf->string, 0));
	TEST_ASSERT_EQUAL_INT('o', *S_get(buf->string, 1));
	TEST_ASSERT_EQUAL_INT('o', *S_get(buf->string, 2));
	TEST_ASSERT_EQUAL_INT('b', *S_get(buf->string, 3));
	TEST_ASSERT_EQUAL_INT('a', *S_get(buf->string, 4));
	TEST_ASSERT_EQUAL_INT('r', *S_get(buf->string, 5));

	StringBuffer_free(buf);
}

static void
test_StringBuffer_cat_with_grow()
{
	StringBuffer* buf = StringBuffer_new(4);

	TEST_ASSERT_BUFFER(buf, "", 4, 0, 4);

	StringBuffer_cat(buf, "foo");
	TEST_ASSERT_BUFFER(buf, "foo", 4, 4, 0);

	StringBuffer_cat(buf, "bar");
	TEST_ASSERT_BUFFER(buf, "foobar", 7, 7, 0);

	StringBuffer_free(buf);
}

static void
test_StringBuffer_cat_with_grow_zero()
{
	StringBuffer* buf = StringBuffer_new(0);

	StringBuffer_cat(buf, "foo");
	TEST_ASSERT_BUFFER(buf, "foo", 4, 4, 0);

	StringBuffer_free(buf);
}

static void
test_StringBuffer_scat()
{
	StringBuffer* buf = StringBuffer_new(0);
	String* s = S_dup("foobar");

	StringBuffer_scat(buf, s);
	TEST_ASSERT_BUFFER(buf, "foobar", 7, 7, 0);

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

	const char* data = "foo";

	fwrite(data, 1, 4, tmpfile);

	size_t read_size = 32;

	// buffer is extended to maximum read size each time before reading

	/* appends 4 chars [foo\0] */
	rewind(tmpfile);
	TEST_ASSERT_EQUAL_INT(4, StringBuffer_fncat(buf, tmpfile, read_size));
	TEST_ASSERT_BUFFER(buf, "foo", read_size + 1, 4, read_size - 3);

	/* appends 3 chars (shift \0) */
	rewind(tmpfile);
	TEST_ASSERT_EQUAL_INT(4, StringBuffer_fncat(buf, tmpfile, read_size));
	TEST_ASSERT_BUFFER(buf, "foofoo", read_size + 4, 7, read_size - 3);

	/* appends 3 chars again (shift \0) */
	rewind(tmpfile);
	TEST_ASSERT_EQUAL_INT(4, StringBuffer_fncat(buf, tmpfile, read_size));
	TEST_ASSERT_BUFFER(buf, "foofoofoo", read_size + 7, 10, read_size - 3);

	/* increments buffer */
	TEST_ASSERT_EQUAL_INT(0, StringBuffer_fncat(buf, tmpfile, read_size));
	TEST_ASSERT_BUFFER(buf, "foofoofoo", read_size + 10, 10, read_size);

	// additional reads do not change anything
	TEST_ASSERT_EQUAL_INT(0, StringBuffer_fncat(buf, tmpfile, read_size));
	TEST_ASSERT_EQUAL_INT(0, StringBuffer_fncat(buf, tmpfile, read_size));
	TEST_ASSERT_EQUAL_INT(0, StringBuffer_fncat(buf, tmpfile, read_size));
	TEST_ASSERT_BUFFER(buf, "foofoofoo", read_size + 10, 10, read_size);

	StringBuffer_free(buf);
	fclose(tmpfile);
	unlink(template);
}

static void
test_StringBuffer_append_grow_offset()
{
	StringBuffer* buf = StringBuffer_new(3);

	TEST_ASSERT_BUFFER(buf, "", 3, 0, 3);

	StringBuffer_append(buf, 0, "foo", 3);
	TEST_ASSERT_BUFFER(buf, "foo", 4, 4, 0);

	StringBuffer_append(buf, 2, "foo", 3);
	TEST_ASSERT_BUFFER(buf, "fofoo", 6, 6, 0);

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

	TEST_ASSERT_EQUAL(pattern_length * iterations + 1, b->string->length);

	S_free(s);
	StringBuffer_free(b);
}

static void
test_StringBuffer_shift()
{
	StringBuffer* buf = StringBuffer_new(12);

	StringBuffer_cat(buf, "foobar");
	TEST_ASSERT_BUFFER(buf, "foobar", 12, 7, 5);

	StringBuffer_shift(buf, 3);
	TEST_ASSERT_BUFFER(buf, "bar", 12, 4, 8);

	StringBuffer_free(buf);
}

static void
test_StringBuffer_clear()
{
	StringBuffer* buf = StringBuffer_new(12);

	StringBuffer_cat(buf, "foo");
	TEST_ASSERT_BUFFER(buf, "foo", 12, 4, 8);

	StringBuffer_clear(buf);
	TEST_ASSERT_BUFFER(buf, "", 12, 0, 12);

	StringBuffer_cat(buf, "bar");
	TEST_ASSERT_BUFFER(buf, "bar", 12, 4, 8);

	StringBuffer_free(buf);
}

static void
test_StringBuffer_printf()
{
	StringBuffer* buf = StringBuffer_new(6);

	StringBuffer_printf(buf, "%s %s", "hello", "world");
	TEST_ASSERT_BUFFER(buf, "hello world", 12, 12, 0);

	StringBuffer_free(buf);
}

static void
test_StringBuffer_aprintf()
{
	// with growing
	StringBuffer* buf = StringBuffer_new(2);
	const char* expected = "1:2,3:4";

	StringBuffer_printf(buf, "%d:%d", 1, 2);
	StringBuffer_aprintf(buf, ",%d:%d", 3, 4);
	TEST_ASSERT_BUFFER(buf, expected, 8, 8, 0);

	StringBuffer_free(buf);

	// without growing
	buf = StringBuffer_new(1024);

	StringBuffer_printf(buf, "%d:%d", 1, 2);
	StringBuffer_aprintf(buf, ",%d:%d", 3, 4);
	TEST_ASSERT_BUFFER(buf, expected, 1024, 8, 1024 - 8);

	StringBuffer_free(buf);
}

int
main()
{
	TEST_BEGIN

	RUN(test_StringBuffer_cat);
	RUN(test_StringBuffer_cat_with_grow);
	RUN(test_StringBuffer_cat_with_grow_zero);
	RUN(test_StringBuffer_scat);
	RUN(test_StringBuffer_fcat);
	RUN(test_StringBuffer_append_grow_offset);
	RUN(test_limits);
	RUN(test_StringBuffer_shift);
	RUN(test_StringBuffer_clear);
	RUN(test_StringBuffer_printf);
	RUN(test_StringBuffer_aprintf);
	TEST_END
}
