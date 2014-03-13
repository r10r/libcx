#include "libcx-base/test.h"
#include "libcx-base/xmalloc.h"
#include "string.h"

NOSETUP

static void test_String_new()
{
	String s = String_new("foo");

	printf("%s\n", s);
	TEST_ASSERT_EQUAL_INT(3, String_space(s));
	String_free(s);
}

static void test_String_append()
{
	String foo = String_new("foo");
	String bar = String_new("bar");
	String foobar = String_append(foo, bar);

	printf("%s\n", foobar);
	TEST_ASSERT_EQUAL_INT(6, String_space(foobar));
	TEST_ASSERT_EQUAL_STRING("foobar", foobar);
	String_free(foobar);
	String_free(bar);
}

static void test_String_append_constant()
{
	String foo = String_new("foo");
	String foobar = String_append_constant(foo, "bar");

	printf("%s\n", foobar);
	TEST_ASSERT_EQUAL_INT(6, String_space(foobar));
	TEST_ASSERT_EQUAL_STRING("foobar", foobar);
	String_free(foobar);
}

static void test_String_append_array()
{
	const char f[] = "foobar";
	String foo = String_new("foo");
	String foobar = String_append_array(foo, &f[3], 3);

	printf("%s\n", foobar);
	TEST_ASSERT_EQUAL_INT(6, String_space(foobar));
	TEST_ASSERT_EQUAL_STRING("foobar", foobar);
	String_free(foobar);
}

static void test_StringPair_new()
{
	Pair *foobar = StringPair_new("foo", "bar");

	TEST_ASSERT_EQUAL_STRING("foo", foobar->key);
	TEST_ASSERT_EQUAL_STRING("bar", foobar->value);
	StringPair_free(foobar);
}

static void test_String_write()
{
	String foo = String_new("foo\n");

	TEST_ASSERT_EQUAL_STRING("foo\n", foo);
	TEST_ASSERT_EQUAL_INT(4, String_write(foo, stdout));
	String_free(foo);
}

static void test_String_append_stream()
{
	char template[] = "/tmp/temp.XXXXXX";
	int fd = mkstemp(template);
	FILE *tmpfile = fdopen(fd, "w+");

	fwrite("foo\n", 4, 1, tmpfile);

	String foo = String_new(NULL);

	rewind(tmpfile);
	foo = String_append_stream(foo, tmpfile, 4);
	TEST_ASSERT_EQUAL_STRING("foo\n", foo);
	TEST_ASSERT_EQUAL_INT(4, String_write(foo, stdout));

	rewind(tmpfile);
	foo = String_append_stream(foo, tmpfile, 4);
	TEST_ASSERT_EQUAL_STRING("foo\nfoo\n", foo);
	TEST_ASSERT_EQUAL_INT(8, String_write(foo, stdout));

	String_free(foo);
	fclose(tmpfile);
	unlink(template);
}

int main()
{
	TEST_BEGIN

	RUN(test_String_new);
	RUN(test_String_append);
	RUN(test_String_append_constant);
	RUN(test_String_append_array);
	RUN(test_StringPair_new);
	RUN(test_String_write);
	RUN(test_String_append_stream);

	TEST_END
}
