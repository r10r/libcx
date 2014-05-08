#ifndef _CX_DEBUG_H
#define _CX_DEBUG_H

#include <string.h>     /* strerror */
#include <errno.h>
#include <stdio.h>      /* fprintf */

#include "base.h"

/*
 * debug message convetions:
 * - first level separator => ' - '
 * - second level separator => ':'
 * - surround strings containing spaces with square brackets '[]'
 * - to emphasize surround element with brackets '()'
 */

#ifdef NDEBUG
#define XDBG(message)
#define XFDBG(format, ...)
#define XFLOG(fmt, ...)
#define XLOG(message)
#else

/*
 * Prints the given message to stderr with debug information if errno is not 0.
 */
#define XDBG(message) \
	fprintf(stderr, "XDBG:(%s):%s:%d - " message "\n", \
		__func__, __FILE__, __LINE__)

#define XFDBG(format, ...) \
	fprintf(stderr, "(%s):%s:%d - " format "\n", \
		__func__, __FILE__, __LINE__, __VA_ARGS__)

#define XFLOG(format, ...) \
	fprintf(stdout, format "\n", __VA_ARGS__)

#define XLOG(message) \
	fprintf(stdout, message "\n")

#endif

#define XEXIT_CODE -1

/*
 * Prints the given message to stderr with debug information if errno is not 0.
 */
// FIXME rename to XERRNO
#define XERR(message) \
	if (errno != 0) \
		fprintf(stderr, "XERR:(%s):%s:%d - %s - errno:%d:[%s]\n", \
			__func__, __FILE__, __LINE__, message, errno, strerror(errno))

#define XFERRNO(format, ...) \
	if (errno != 0) \
		fprintf(stderr, "XERR:(%s):%s:%d - errno:%d:[%s] - " format "\n", \
			__func__, __FILE__, __LINE__, errno, strerror(errno), __VA_ARGS__)

#define XFERR(format, ...) \
	fprintf(stderr, "XERR:(%s):%s:%d - " format "\n", \
		__func__, __FILE__, __LINE__, __VA_ARGS__)

#ifdef NASSERT
#define XCHECK(condition, message)
#define XCHECK_EQUALS_INT(expected, actual, message)
#define XFCHECK(condition, format, ...)
#define XASSERT(condition, message)
#define XFASSERT(condition, format, ...)
#else

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
