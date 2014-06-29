#ifndef _CX_ERRNO_H
#define _CX_ERRNO_H

extern __thread int cx_errno;
#define set_cx_errno(err) (cx_errno = (err))
//#define cx_errno (cx_errno)

#endif
