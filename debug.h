#ifndef _DEBUG_H
#define _DEBUG_H

#include <string.h> /* strerror */
#include <errno.h>

/*
 * debug message convetions:
 * - first level separator => ' - '
 * - second level separator => ':'
 * - surround strings containing spaces with square brackets '[]'
 * - to emphasize surround element with brackets '()'
 */

#ifdef NDEBUG
#define XDBG(message)
#define XFLOG(fmt, ...)
#define XLOG(message)
#else
/*
 * Prints the given message to stderr with debug information if errno is not 0.
 */
#define XDBG(message) \
	fprintf(stderr, "XDBG:[%s] - (%s):%s:%d\n", \
		message, __func__, __FILE__, __LINE__)

#define XFLOG(fmt, ...) \
	fprintf(stdout, fmt, __VA_ARGS__)

#define XLOG(message) \
	fprintf(stdout, message)

#endif

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
#define XASSERT(message, expression) \
	if (!(expression)) \
		fprintf(stderr, "XASSERT:[%s] - (%s):%s:%d\n", \
			message, __func__, __FILE__, __LINE__)
#endif
