#ifndef _CX_ERRNO_H
#define _CX_ERRNO_H

#include <stdio.h>
#include <assert.h>

#include "debug.h"
#include "base.h"

#define CX_ERR_OK 0

extern __thread int CX_HIDDEN(err_code);

#define cx_ferr_set(_code, err_fmt, ...) \
	{ \
		cx_printf("CX_ERR" CX_DBG_FMT "code:%d message:[" err_fmt "]\n",  \
			  __func__, __FILE__, __LINE__, _code, __VA_ARGS__); \
		assert(CX_HIDDEN(err_code) == CX_ERR_OK); \
		CX_HIDDEN(err_code) = _code; \
	}

#define cx_err_set(_code, err_msg) \
	cx_ferr_set(_code, "%s", err_msg)

#define cx_err_clear() \
	{ \
		cx_printf("CX_ERR" CX_DBG_FMT "clear error (code:%d)\n", \
			  __func__, __FILE__, __LINE__, CX_HIDDEN(err_code)); \
		CX_HIDDEN(err_code) = CX_ERR_OK; \
	}

#define cx_err_code CX_HIDDEN(err_code)


#endif
