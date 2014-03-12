#include "libcx-base/test.h"

NOSETUP

static void test_XASSERT()
{
	XASSERT("0 equals 0", 0 == 0);
}


static void test_XCHECK()
{
	XCHECK("0 equals 0", 0 == 0);
	XCHECK("0 does not equal 1", 0 == 1);
}

static void test_XERR()
{
	XERR("You should not see this");
	errno = EPERM;
	XERR(strerror(errno));
}

static void test_XDBG()
{
	XDBG("You should see this");
}


int main()
{
	TEST_BEGIN

	RUN(test_XASSERT);
	RUN(test_XCHECK);
	RUN(test_XERR);
	RUN(test_XDBG);

	TEST_END
}
