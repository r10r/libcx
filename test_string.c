#include "libcx-base/test.h"
#include "libcx-base/xmalloc.h"
#include "string.h"

NOSETUP

static void test_String_free()
{
	String_free(NULL);
}

static void test_String_new()
{
	String s = String_new("foo");

	TEST_ASSERT_EQUAL_STRING("foo", s);
	TEST_ASSERT_EQUAL_INT(3, String_available(s));
	String_free(s);
}

static void test_String_init()
{
	String s = String_init(NULL, 3);

	TEST_ASSERT_EQUAL_STRING("", s);
	TEST_ASSERT_EQUAL_INT(3, String_available(s));

	String_append_constant(s, "foo");
	TEST_ASSERT_EQUAL_STRING("foo", s);
	TEST_ASSERT_EQUAL_INT(3, String_available(s));

	String_free(s);
}

static void test_String_append()
{
	String foo = String_new("foo");
	String bar = String_new("bar");
	String null_s = String_new(NULL);
	String foobar = String_append(foo, bar);

	TEST_ASSERT_NULL(String_append(NULL, bar));
	TEST_ASSERT_EQUAL_PTR(foo, String_append(foo, NULL));
	TEST_ASSERT_EQUAL_PTR(foo, String_append(foo, null_s));

	TEST_ASSERT_EQUAL_INT(6, String_available(foobar));
	TEST_ASSERT_EQUAL_STRING("foobar", foobar);

	String_free(foobar);
	String_free(bar);
	String_free(null_s);
}

static void test_String_append_constant()
{
	String foo = String_new("foo");

	foo = String_append_constant(foo, "bar");

	TEST_ASSERT_NULL(String_append_constant(NULL, foo));
	TEST_ASSERT_EQUAL_PTR(foo, String_append_constant(foo, NULL));
	TEST_ASSERT_EQUAL_PTR(foo, String_append_constant(foo, ""));

	TEST_ASSERT_EQUAL_INT(6, String_available(foo));
	TEST_ASSERT_EQUAL_STRING("foobar", foo);
	String_free(foo);
}

static void test_String_append_constant_without_grow()
{
	String foo = String_init(NULL, 1024);

	String_append_constant(foo, "foo");

	TEST_ASSERT_EQUAL_INT(1024, String_available(foo));
	TEST_ASSERT_EQUAL_INT(1024 - 3, String_unused(foo));
	TEST_ASSERT_EQUAL_STRING("foo", foo);
	String_free(foo);
}

static void test_String_append_array()
{
	const char f[] = "foobar";
	String foo = String_new("foo");
	String foobar = String_append_array(foo, &f[3], 3);

	TEST_ASSERT_NULL(String_append_array(NULL, &f[0], 6));
	TEST_ASSERT_EQUAL_PTR(foo, String_append_array(foo, NULL, 66));
	TEST_ASSERT_EQUAL_PTR(foo, String_append_array(foo, &f[0], 0));

	TEST_ASSERT_EQUAL_INT(6, String_available(foobar));
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

static void test_String_last()
{
	String s = String_new("foo");
	String null_s = String_new(NULL);

	TEST_ASSERT_EQUAL_PTR(&s[2], String_last(s));
	TEST_ASSERT_NULL(String_last(null_s));
	String_free(s);
	String_free(null_s);
}

int main()
{
	TEST_BEGIN

	RUN(test_String_free);
	RUN(test_String_new);
	RUN(test_String_init);
	RUN(test_String_append);
	RUN(test_String_append_constant);
	RUN(test_String_append_constant_without_grow);
	RUN(test_String_append_array);
	RUN(test_StringPair_new);
	RUN(test_String_write);
	RUN(test_String_append_stream);
	RUN(test_String_last);

	TEST_END
}
