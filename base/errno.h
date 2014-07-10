#ifndef _CX_ERRNO_H
#define _CX_ERRNO_H

#include <stdio.h>

#define CX_HIDDEN(var) \
	___cx_ ## var ## ___

#define CX_ERR_OK 0
#define CX_ERR_DBG_FMT "CX_ERR" CX_DBG_FMT "token:%s code:%d message:[%s]"

extern __thread int CX_HIDDEN(err_code);

#define cx_ferr_set(_code, err_fmt, ...) \
	{ \
		cx_printf("CX_ERRNO" CX_DBG_FMT "token:%s code:%d message:[" err_fmt "]\n",  \
			  __func__, __FILE__, __LINE__, cx_next_uid(), _code, __VA_ARGS__); \
		assert(CX_HIDDEN(err_code) == CX_ERR_OK); \
		CX_HIDDEN(err_code) = _code; \
	}

#define cx_err_set(_code, err_msg) \
	cx_ferr_set(_code, "%s", err_msg)

#define cx_err_clear() \
	{ \
		cx_printf("CX_ERRNO" CX_DBG_FMT "clear code:%d\n", __func__, __FILE__, __LINE__, CX_HIDDEN(err_code)); \
		CX_HIDDEN(err_code) = CX_ERR_OK; \
	}

#define cx_err_code CX_HIDDEN(err_code)


#include <pthread.h>
#include <time.h>
#include <stdint.h>

#define CX_UID_LENGTH   (10 + 10 + 1) /* timestamp (uint32_t) + counter (uint32_t) + \0 */

uint32_t
cx_uid_counter(void);

/*
 * Increment the uid counter.
 *
 * @return the current uid
 */
const char*
cx_next_uid(void);

const char*
cx_uid(void);


#endif
