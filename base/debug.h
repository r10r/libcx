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
#define CX_DBG_FMT ":(%s) %s %d - "   /* function, file, line */

/* consider using write ? (should be atomic up to PIPE_BUFF_MAX) */
#define cx_printf(format, ...) \
	{ flockfile(stderr); fprintf(stderr, format, __VA_ARGS__); funlockfile(stderr); }

#ifdef _CX_MULTI_THREADED
#include <pthread.h>

#define cx_log(format, ...) \
	{ flockfile(stderr); fprintf(stderr, "thread[%d] " format "\n", (int)pthread_self(), __VA_ARGS__); funlockfile(stderr); }
#else
#define cx_log(format, ...) \
	{ flockfile(stderr); fprintf(stderr, format "\n", __VA_ARGS__); funlockfile(stderr); }
#endif

#ifndef _CX_DEBUG
#define XDBG(message) UNUSED(message)
#define XFDBG(format, ...) UNUSED(format)
#else

/*
 * Prints the given message to stderr with debug information if errno is not 0.
 */
#define XDBG(message) \
	cx_log("XDBG" CX_DBG_FMT message, \
	       __func__, __FILE__, __LINE__)

#define XFDBG(format, ...) \
	cx_log("XDBG" CX_DBG_FMT format, \
	       __func__, __FILE__, __LINE__, __VA_ARGS__)

#endif

#define XEXIT_CODE -1


#define XFLOG(format, ...) \
	cx_log(format, __VA_ARGS__)

#define XLOG(message) \
	cx_log("%s", message)

#define XERRNO(message) \
	cx_log("XERR" CX_DBG_FMT "%s errno:%d:[%s]", \
	       __func__, __FILE__, __LINE__, message, errno, strerror(errno))

#define XFERRNO(format, ...) \
	cx_log("XERR" CX_DBG_FMT "errno:%d:[%s] - " format, \
	       __func__, __FILE__, __LINE__, errno, strerror(errno), __VA_ARGS__)

#define XERR(message) \
	cx_log("XERR" CX_DBG_FMT message, \
	       __func__, __FILE__, __LINE__)

#define XFERR(format, ...) \
	cx_log("XERR" CX_DBG_FMT format, \
	       __func__, __FILE__, __LINE__, __VA_ARGS__)

#define XWARN(message) \
	cx_log("XWARN" CX_DBG_FMT "%s", \
	       __func__, __FILE__, __LINE__, message)

#define XFWARN(format, ...) \
	cx_log("XWARN" CX_DBG_FMT format, \
	       __func__, __FILE__, __LINE__, __VA_ARGS__)

#endif
