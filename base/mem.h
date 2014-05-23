#ifndef _CX_MEMDBG_H
#define _CX_MEMDBG_H

#include <stdio.h>      /* printf */
#include <stdlib.h>     /* calloc */
#include <string.h>     /* strdup */

#ifndef _CX_DEBUG_MEM

#define cx_alloc(size) calloc(1, size)
#define cx_free(ptr) free(ptr)
#define cx_strdup(s) strdup(s)

#else

#define MEMDBG_PREFIX "__MEMDBG"
#define ALLOC_TOKEN '+'
#define FREE_TOKEN '-'

static inline void
cx_mem_dbg(void* ptr, char token, const char* file, int line, const char* func, size_t count, size_t eltsize)
{
	flockfile(stderr);
	if (ptr)
		fprintf(stderr, MEMDBG_PREFIX " %p %c %s %s %d %zu\n", ptr, token, file, func, line, count * eltsize);
	else
		/* memory allocation error */
		fprintf(stderr, MEMDBG_PREFIX " 0x0 %c %s %s %d 0\n", token, file, func, line);
	funlockfile(stderr);
}

static inline void*
cx_calloc_dbg(size_t count, size_t eltsize, const char* file, int line, const char* func)
{
	void* ptr = calloc(count, eltsize);

	cx_mem_dbg(ptr, ALLOC_TOKEN, file, line, func, count, eltsize);
	return ptr;
}

static inline void
cx_free_dbg(void* ptr, const char* file, int line, const char* func)
{
	cx_mem_dbg(ptr, FREE_TOKEN, file, line, func, 0, 0);
	free(ptr);
}

static inline char*
cx_strdup_dbg(const char* s, const char* file, int line, const char* func)
{
	char* s_dup = strdup(s);

	cx_mem_dbg((void*)s_dup, ALLOC_TOKEN, file, line, func, 1, strlen(s) + 1);
	return s_dup;
}

#define cx_alloc(eltsize) \
	cx_calloc_dbg(1, eltsize, __FILE__, __LINE__, __func__)

#define cx_free(ptr) \
	cx_free_dbg(ptr, __FILE__, __LINE__, __func__)

#define cx_strdup(s) \
	cx_strdup_dbg(s, __FILE__, __LINE__, __func__)

#endif
#endif
