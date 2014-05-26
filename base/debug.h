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

#ifndef _CX_DEBUG
#define XDBG(message) UNUSED(message)
#define XFDBG(format, ...) UNUSED(format)
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

#endif

#define XEXIT_CODE -1


#define XFLOG(format, ...) \
	fprintf(stdout, format "\n", __VA_ARGS__)

#define XLOG(message) \
	fprintf(stdout, message "\n")

/*
 * Prints the given message to stderr with debug information if errno is not 0.
 */
// FIXME rename to XERRNO
#define XERRNO(message) \
	fprintf(stderr, "XERR:(%s):%s:%d - %s - errno:%d:[%s]\n", \
		__func__, __FILE__, __LINE__, message, errno, strerror(errno))

#define XFERRNO(format, ...) \
	fprintf(stderr, "XERR:(%s):%s:%d - errno:%d:[%s] - " format "\n", \
		__func__, __FILE__, __LINE__, errno, strerror(errno), __VA_ARGS__)

#define XERR(message) \
	fprintf(stderr, "XERR:(%s):%s:%d - " message "\n", \
		__func__, __FILE__, __LINE__)

#define XFERR(format, ...) \
	fprintf(stderr, "XERR:(%s):%s:%d - " format "\n", \
		__func__, __FILE__, __LINE__, __VA_ARGS__)

#define XWARN(message) \
	fprintf(stderr, "XWARN:(%s):%s:%d - %s \n", \
		__func__, __FILE__, __LINE__, message)

#define XFWARN(format, ...) \
	fprintf(stderr, "XWARN:(%s):%s:%d - " format "\n", \
		__func__, __FILE__, __LINE__, __VA_ARGS__)

#endif
