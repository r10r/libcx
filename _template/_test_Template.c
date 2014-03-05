#include "test/test.h"

NOSETUP

void test_Something()
{
	TEST_IGNORE_MESSAGE("Not implemented");
}

int main()
{
	TEST_BEGIN

	RUN(test_Something);

	TEST_END
}
