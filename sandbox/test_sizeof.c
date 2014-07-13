#include <libcx/base/test.h>

static void
test_sizeof_void_ptr()
{
	TEST_ASSERT_EQUAL_INT(sizeof(void*), sizeof(char*));
}

union foo
{
	int integer;
	const char* string;
	void* obj;
};

static void
test_union_typecast()
{
	union foo foobar;

	foobar.integer = 99;

	// from a larger to a smaller value is always possible
	int intval = (int)foobar.obj;

	TEST_ASSERT_EQUAL(99, intval);

	// but casting from a smaller type throws a compiler error
//	foobar.string = "hello world";
//	TEST_ASSERT_EQUAL_STRING("hello world", (const char*)foobar.integer);
}

int
main()
{
	TEST_BEGIN
	RUN(test_sizeof_void_ptr);
	RUN(test_union_typecast);
	TEST_END
}
