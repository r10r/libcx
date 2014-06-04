#ifndef _CX_DEBUG_H
#define _CX_DEBUG_H

#include <string.h>             /* strerror */
#include <errno.h>
#include <stdio.h>              /* fprintf, flockfile, funlockfile */

/*
 * debug message convetions:
 * - first level separator => ' - '
 * - second level separator => ':'
 * - surround strings containing spaces with square brackets '[]'
 * - to emphasize surround element with brackets '()'
 */
#define DFMT ":(%s) %s %d - "   /* function, method, line */


/* consider using write ? (should be atomic up to PIPE_BUFF_MAX) */
#define cx_log(format, ...) \
	{ flockfile(stderr); fprintf(stderr, format "\n", __VA_ARGS__); funlockfile(stderr); }

#ifndef _CX_DEBUG
#define XDBG(message) UNUSED(message)
#define XFDBG(format, ...) UNUSED(format)
#else

/*
 * Prints the given message to stderr with debug information if errno is not 0.
 */
#define XDBG(message) \
	cx_log("XDBG" DFMT message, \
	       __func__, __FILE__, __LINE__)

#define XFDBG(format, ...) \
	cx_log("XDBG" DFMT format, \
	       __func__, __FILE__, __LINE__, __VA_ARGS__)

#endif

#define XEXIT_CODE -1


#define XFLOG(format, ...) \
	cx_log(format, __VA_ARGS__)

#define XLOG(message) \
	cx_log("%s", message)

#define XERRNO(message) \
	cx_log("XERR" DFMT "%s errno:%d:[%s]", \
	       __func__, __FILE__, __LINE__, message, errno, strerror(errno))

#define XFERRNO(format, ...) \
	cx_log("XERR" DFMT "errno:%d:[%s] - " format, \
	       __func__, __FILE__, __LINE__, errno, strerror(errno), __VA_ARGS__)

#define XERR(message) \
	cx_log("XERR" DFMT message, \
	       __func__, __FILE__, __LINE__)

#define XFERR(format, ...) \
	cx_log("XERR" DFMT format, \
	       __func__, __FILE__, __LINE__, __VA_ARGS__)

#define XWARN(message) \
	cx_log("XWARN" DFMT "%s", \
	       __func__, __FILE__, __LINE__, message)

#define XFWARN(format, ...) \
	cx_log("XWARN" DFMT format, \
	       __func__, __FILE__, __LINE__, __VA_ARGS__)

#endif
