#include "base/test.h"
#include "string_buffer.h"

#define TEST_ASSERT_BUFFER(buf, s, blen, used, unused) \
	TEST_ASSERT_EQUAL_STRING(s, StringBuffer_value(buf)); \
	TEST_ASSERT_EQUAL_INT(blen, StringBuffer_length(buf)); \
	TEST_ASSERT_EQUAL_INT(used, StringBuffer_used(buf)); \
	TEST_ASSERT_EQUAL_INT(unused, StringBuffer_unused(buf));

static void
test_StringBuffer_empty()
{
	StringBuffer* buf = StringBuffer_new(0);

	TEST_ASSERT_EQUAL_INT(0, StringBuffer_used(buf));
	TEST_ASSERT_EQUAL_STRING("", StringBuffer_value(buf));

	StringBuffer_free(buf);
}

static void
test_StringBuffer_cat()
{
	StringBuffer* buf = StringBuffer_new(1024);

	TEST_ASSERT_BUFFER(buf, "", 1024, 0, 1024);

	StringBuffer_cat(buf, "foo");
	TEST_ASSERT_BUFFER(buf, "foo", 1024, 3, 1024 - 3);

	StringBuffer_cat(buf, "bar");
	TEST_ASSERT_BUFFER(buf, "foobar", 1024, 6, 1024 - 6);

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
	TEST_ASSERT_BUFFER(buf, "foo", 4, 3, 1);

	StringBuffer_cat(buf, "bar");
	TEST_ASSERT_BUFFER(buf, "foobar", 6, 6, 0);

	StringBuffer_free(buf);
}

static void
test_StringBuffer_cat_with_grow_zero()
{
	StringBuffer* buf = StringBuffer_new(0);

	StringBuffer_cat(buf, "foo");
	TEST_ASSERT_BUFFER(buf, "foo", 3, 3, 0);

	StringBuffer_free(buf);
}

static void
test_StringBuffer_scat()
{
	StringBuffer* buf = StringBuffer_new(0);
	String* s = S_dup("foobar");

	StringBuffer_scat(buf, s);
	TEST_ASSERT_BUFFER(buf, "foobar", 6, 6, 0);

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

	/* write string without \0 terminator */
	fwrite(data, 1, 3, tmpfile);

	ssize_t read_size = 32;

	// buffer is extended to maximum read size each time before reading

	/* appends 4 chars [foo\0] */
	rewind(tmpfile);
	TEST_ASSERT_EQUAL_INT(3, StringBuffer_fncat(buf, tmpfile, read_size));
	TEST_ASSERT_BUFFER(buf, "foo", read_size, 3, read_size - 3);

	/* appends 3 chars (shift \0) */
	rewind(tmpfile);
	TEST_ASSERT_EQUAL_INT(3, StringBuffer_fncat(buf, tmpfile, read_size));
	TEST_ASSERT_BUFFER(buf, "foofoo", read_size + 3, 6, read_size - 3);

	/* appends 3 chars again (shift \0) */
	rewind(tmpfile);
	TEST_ASSERT_EQUAL_INT(3, StringBuffer_fncat(buf, tmpfile, read_size));
	TEST_ASSERT_BUFFER(buf, "foofoofoo", read_size + 6, 9, read_size - 3);

	/* increments buffer */
	TEST_ASSERT_EQUAL_INT(0, StringBuffer_fncat(buf, tmpfile, read_size));
	TEST_ASSERT_BUFFER(buf, "foofoofoo", read_size + 9, 9, read_size);

	// additional reads do not change anything
	TEST_ASSERT_EQUAL_INT(0, StringBuffer_fncat(buf, tmpfile, read_size));
	TEST_ASSERT_EQUAL_INT(0, StringBuffer_fncat(buf, tmpfile, read_size));
	TEST_ASSERT_EQUAL_INT(0, StringBuffer_fncat(buf, tmpfile, read_size));
	TEST_ASSERT_BUFFER(buf, "foofoofoo", read_size + 9, 9, read_size);

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
	TEST_ASSERT_BUFFER(buf, "foo", 3, 3, 0);

	StringBuffer_append(buf, 2, "foo", 3);
	TEST_ASSERT_BUFFER(buf, "fofoo", 5, 5, 0);

	StringBuffer_free(buf);
}

/* TODO check error codes */
static void
test_StringBuffer_shift()
{
	StringBuffer* buf = StringBuffer_new(12);

	StringBuffer_cat(buf, "foobar");
	TEST_ASSERT_BUFFER(buf, "foobar", 12, 6, 6);

	/* shift nothing should return 0 */
	StringBuffer_shift(buf, 0);
	TEST_ASSERT_BUFFER(buf, "foobar", 12, 6, 6);

	StringBuffer_shift(buf, 3);
	TEST_ASSERT_BUFFER(buf, "bar", 12, 3, 9);

	/* shift to much should do nothing and return CX_ERR */
	StringBuffer_shift(buf, 6);
	TEST_ASSERT_BUFFER(buf, "bar", 12, 3, 9);

	/* shift the remaining content*/
	StringBuffer_shift(buf, 3);
	TEST_ASSERT_BUFFER(buf, "", 12, 0, 12);

	StringBuffer_free(buf);
}

static void
test_StringBuffer_clear()
{
	StringBuffer* buf = StringBuffer_new(12);

	StringBuffer_cat(buf, "foo");
	TEST_ASSERT_BUFFER(buf, "foo", 12, 3, 9);

	StringBuffer_clear(buf);
	TEST_ASSERT_BUFFER(buf, "", 12, 0, 12);

	StringBuffer_cat(buf, "bar");
	TEST_ASSERT_BUFFER(buf, "bar", 12, 3, 9);

	StringBuffer_free(buf);
}

static void
test_StringBuffer_printf()
{
	StringBuffer* buf = StringBuffer_new(6);

	StringBuffer_printf(buf, "%s %s", "hello", "world");
	TEST_ASSERT_BUFFER(buf, "hello world", 11, 11, 0);

	StringBuffer_free(buf);
}

static void
test_StringBuffer_aprintf()
{
	StringBuffer* buf;
	const char* expected = "1:2,3:4";

	// without growing
	buf = StringBuffer_new(1024);
	StringBuffer_printf(buf, "%d:%d", 1, 2);
	StringBuffer_aprintf(buf, ",%d:%d", 3, 4);
	TEST_ASSERT_BUFFER(buf, expected, 1024, 7, 1024 - 7);
	StringBuffer_free(buf);


	// with growing
	buf = StringBuffer_new(2);
	StringBuffer_printf(buf, "%d:%d", 1, 2);
	TEST_ASSERT_BUFFER(buf, "1:2", 3, 3, 0);
	StringBuffer_aprintf(buf, ",%d:%d", 3, 4);
	TEST_ASSERT_BUFFER(buf, expected, 7, 7, 0);
	StringBuffer_free(buf);
}

static void
test_StringBuffer_from_string()
{
	StringBuffer* buffer = StringBuffer_from_string(S_dup("foobar"));

	TEST_ASSERT_EQUAL_STRING("foobar", StringBuffer_value(buffer));
	StringBuffer_free(buffer);
}

static void
test_StringBuffer_from_printf()
{
	StringBuffer* buffer = StringBuffer_from_printf(1024, "%s%s", "foo", "bar");

	TEST_ASSERT_EQUAL_STRING("foobar", StringBuffer_value(buffer));
	StringBuffer_free(buffer);
}

static void
test_StringBuffer_at()
{
	StringBuffer* buffer = StringBuffer_from_printf(1024, "%s%s", "foo", "bar");

	TEST_ASSERT_EQUAL_STRING("foobar", StringBuffer_at(buffer, -6));
	TEST_ASSERT_EQUAL_STRING("bar", StringBuffer_at(buffer, 3));

	TEST_ASSERT_NULL(StringBuffer_at(buffer, 6));
	TEST_ASSERT_NULL(StringBuffer_at(buffer, -7));

	StringBuffer_free(buffer);
}

static void
test_StringBuffer_append_nullbytes()
{
	StringBuffer* buffer = StringBuffer_new(0);

	const char data[] = { 0x0, 0x0 };

	StringBuffer_ncat(buffer, data, 2);
	TEST_ASSERT_EQUAL_INT(2, StringBuffer_used(buffer));
	TEST_ASSERT_EQUAL_INT(0x0, *S_get(buffer->string, 0));
	TEST_ASSERT_EQUAL_INT(0x0, *S_get(buffer->string, 1));

	StringBuffer_free(buffer);
}

int
main()
{
	TEST_BEGIN

	RUN(test_StringBuffer_empty);
	RUN(test_StringBuffer_cat);
	RUN(test_StringBuffer_cat_with_grow);
	RUN(test_StringBuffer_cat_with_grow_zero);
	RUN(test_StringBuffer_scat);
	RUN(test_StringBuffer_fcat);
	RUN(test_StringBuffer_append_grow_offset);
	RUN(test_StringBuffer_shift);
	RUN(test_StringBuffer_clear);
	RUN(test_StringBuffer_printf);
	RUN(test_StringBuffer_aprintf);
	RUN(test_StringBuffer_from_string);
	RUN(test_StringBuffer_from_printf);
	RUN(test_StringBuffer_at);
	RUN(test_StringBuffer_append_nullbytes);
	TEST_END
}
