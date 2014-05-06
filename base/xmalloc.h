#ifndef _CX_XMALLOC_H
#define _CX_XMALLOC_H

#include "stddef.h"     /* size_t */
#include <stdlib.h>     /* malloc / free */
#include <stdio.h>      /* printf */

#include "debug.h"

void*
d_xmalloc(size_t size, const char* func, const char* file, int line);

void*
d_xcalloc(size_t count, size_t size, const char* func, const char* file, int line);

void*
d_xrealloc(void* ptr, size_t size, const char* func, const char* file, int line);

void
d_xfree(void* ptr, const char* func, const char* file, int line);


#ifndef _XMALLOC_DISABLED

#define malloc(size) \
	d_xmalloc(size, __func__, __FILE__, __LINE__)

#define calloc(count, size) \
	d_xcalloc(count, size, __func__, __FILE__, __LINE__)

#define realloc(ptr, size) \
	d_xrealloc(ptr, size, __func__, __FILE__, __LINE__)

#define free(ptr) \
	d_xfree(ptr, __func__, __FILE__, __LINE__)

#endif

#endif
