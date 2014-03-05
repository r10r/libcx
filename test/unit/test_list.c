#include "test.h"
#include "list.h"

NOSETUP

void test_Something()
{
	TEST_ASSERT_EQUAL_STRING("list", list());
}

int main()
{
	TEST_BEGIN

	RUN(test_Something);

	TEST_END
}
