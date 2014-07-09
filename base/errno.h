#ifndef _CX_ERRNO_H
#define _CX_ERRNO_H

#include <stdio.h>

extern __thread int cx_errno;

void
dbg_cx_errno_set(int err, const char* file, int line, const char* func);

#ifdef _CX_DEBUG
#define cx_errno_set(err) (dbg_cx_errno_set(err, __FILE__, __LINE__, __func__))
#else
#define cx_errno_set(err) (cx_errno = (err))
#endif


#endif
