#ifndef _CX_ERRNO_H
#define _CX_ERRNO_H

#include <stdio.h>

extern __thread int cx_errno;

void
dbg_set_cx_errno(int err, const char* file, int line, const char* func);

#ifdef _CX_DEBUG
#define set_cx_errno(err) (dbg_set_cx_errno(err, __FILE__, __LINE__, __func__))
#else
#define set_cx_errno(err) (cx_errno = (err))
#endif

//#define cx_errno (cx_errno)

#endif
