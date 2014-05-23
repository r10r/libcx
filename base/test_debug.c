#include "test.h"

static void
test_XASSERT()
{
	XASSERT(0 == 0,
		"0 equals 0");
}

static void
test_XCHECK()
{
	XCHECK(0 == 0,
	       "0 equals 0");
	XCHECK(0 == 1,
	       "0 does not equal 1");
}

static void
test_XFCHECK()
{
	XFCHECK(1 == 3,
		"%d equals %d", 1, 3);
}

static void
test_XERR()
{
	XERR("You should not see this");
	errno = EPERM;
	char* err = strerror(errno);
	XERR(err);
}

static void
test_XDBG()
{
	XDBG("You should see this");
}

int
main()
{
	TEST_BEGIN

	RUN(test_XASSERT);
	RUN(test_XCHECK);
	RUN(test_XFCHECK);
	RUN(test_XERR);
	RUN(test_XDBG);

	TEST_END
}
