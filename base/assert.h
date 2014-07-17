#ifndef _CX_ASSERT_H
#define _CX_ASSERT_H

#include <assert.h>

#ifndef _CX_ASSERT

#define cx_assert(cond) (void)(cond)

#define XCHECK(condition, message) UNUSED(condition)
#define XCHECK_EQUALS_INT(expected, actual, message) UNUSED(expected)
#define XFCHECK(condition, format, ...) UNUSED(condition)
#define XASSERT(condition, message) UNUSED(condition)
#define XFASSERT(condition, format, ...) UNUSED(condition)

#else

#define cx_assert(cond) assert(cond)

/*
 * Print message to stderr when assertion fails.
 */
#define XCHECK(condition, message) \
	if (!(condition)) \
		fprintf(stderr, "XCHECK:(%s):%s:%d - %s\n", \
			__func__, __FILE__, __LINE__, message)

#define XCHECK_EQUALS_INT(expected, actual, message) \
	if (expected != actual) \
		fprintf(stderr, "XFCHECK:(%s):%s:%d - [%s (expected %d, was %d)]\n", \
			__func__, __FILE__, __LINE__, message, expected, actual)

#define XFCHECK(condition, format, ...) \
	if (!(condition)) \
		fprintf(stderr, "XFCHECK:(%s):%s:%d - " format "\n", \
			__func__, __FILE__, __LINE__, __VA_ARGS__); \


/*
 * Print message to stderr and exit with XEXIT_CODE when assertion fails.
 */
#define XASSERT(condition, message) \
	if (!(condition)) \
	{ \
		fprintf(stderr, "XASSERT:(%s):%s:%d - " message "\n", \
			__func__, __FILE__, __LINE__); \
		exit(XEXIT_CODE); \
	}

#define XFASSERT(condition, format, ...) \
	if (!(condition)) { \
		fprintf(stderr, "XASSERT:(%s):%s:%d - " format "\n", \
			__func__, __FILE__, __LINE__, __VA_ARGS__); \
		exit(XEXIT_CODE); \
	}

#endif

#endif
