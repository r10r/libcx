#ifndef _CX_DEBUG_H
#define _CX_DEBUG_H

#include <string.h>     /* strerror */
#include <errno.h>
#include <stdio.h>      /* fprintf */

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
	fprintf(stderr, "XDBG:[%s] - (%s):%s:%d\n", \
		message, __func__, __FILE__, __LINE__)

#define XFDBG(format, ...) \
	fprintf(stderr, "(%s):%s:%d " format, \
		__func__, __FILE__, __LINE__, __VA_ARGS__)

#define XFLOG(format, ...) \
	fprintf(stdout, format, __VA_ARGS__)

#define XLOG(message) \
	fprintf(stdout, message)

#endif

#define XEXIT_CODE -1

#ifdef NASSERT
#define XERR(message)
#define XCHECK(condition, message)
#define XCHECK_EQUALS_INT(expected, actual, message)
#define XFCHECK(condition, format, ...)
#define XASSERT(condition, message)
#define XFASSERT(condition, format, ...)
#else

/*
 * Prints the given message to stderr with debug information if errno is not 0.
 */
#define XERR(message) \
	if (errno != 0) \
		fprintf(stderr, "XERR:[%s] - errno:%d:[%s] - (%s):%s:%d\n", \
			message, errno, strerror(errno), __func__, __FILE__, __LINE__)

/*
 * Print message to stderr when assertion fails.
 */
#define XCHECK(condition, message) \
	if (!(condition)) \
		fprintf(stderr, "XCHECK:[%s] - (%s):%s:%d\n", \
			message, __func__, __FILE__, __LINE__)

#define XCHECK_EQUALS_INT(expected, actual, message) \
	if (expected != actual) \
		fprintf(stderr, "XFCHECK:[%s (expected %d, was %d)] - (%s):%s:%d\n", \
			message, expected, actual, __func__, __FILE__, __LINE__)

#define XFCHECK(condition, format, ...) \
	if (!(condition)) \
	{ \
		fprintf(stderr, "XFCHECK:["); \
		fprintf(stderr, format, __VA_ARGS__); \
		fprintf(stderr, "] - (%s):%s:%d\n", \
			__func__, __FILE__, __LINE__); \
	}

/*
 * Print message to stderr and exit with XEXIT_CODE when assertion fails.
 */
#define XASSERT(condition, message) \
	if (!(condition)) { \
		fprintf(stderr, "XASSERT:[%s] - (%s):%s:%d\n", \
			message, __func__, __FILE__, __LINE__); \
		exit(XEXIT_CODE); \
	}

#define XFASSERT(condition, format, ...) \
	if (!(condition)) { \
		fprintf(stderr, "XASSERT:(%s):%s:%d - " format, \
			__func__, __FILE__, __LINE__, __VA_ARGS__); \
		exit(XEXIT_CODE); \
	}

#endif

#endif
