#include "libcx-base/test.h"
#include "libcx-base/debug.h"

NOSETUP

void test_XASSERT()
{
	XASSERT("0 equals 0", 0 == 0);
	XASSERT("0 does not equal 1", 0 == 1);
}

void test_XERR()
{
	XERR("You should not see this");
	errno = EPERM;
	XERR(strerror(errno));
}

void test_XDBG()
{
	XDBG("You should see this");
}


int main()
{
	TEST_BEGIN

	RUN(test_XASSERT);
	RUN(test_XERR);
	RUN(test_XDBG);

	TEST_END
}
