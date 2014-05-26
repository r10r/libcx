#include "../base/test.h"

static void
test_sizeof_void_ptr()
{
	TEST_ASSERT_EQUAL_INT(sizeof(void*), sizeof(char*));
}

int
main()
{
	TEST_BEGIN
	RUN(test_sizeof_void_ptr);
	TEST_END
}
