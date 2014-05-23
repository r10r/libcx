#include "mem.h"

void*
malloc(size_t eltsize);

void*
calloc(size_t count, size_t eltsize);

void
free(void* ptr);

char*
strdup(const char* s);

void*
malloc(size_t eltsize)
{
	return cx_calloc_dbg(1, eltsize, __FILE__, __LINE__, __func__);
}

void*
calloc(size_t count, size_t eltsize)
{
	return cx_calloc_dbg(count, eltsize, __FILE__, __LINE__, __func__);
}

void
free(void* ptr)
{
	cx_free_dbg(ptr, __FILE__, __LINE__, __func__);
}

char*
strdup(const char* s)
{
	return cx_strdup_dbg(s, __FILE__, __LINE__, __func__);
}
