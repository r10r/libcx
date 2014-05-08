/* do not replace malloc/realloc/free with macros here */
#define _XMALLOC_DISABLED
#include "xmalloc.h"

void*
d_xmalloc(size_t size, const char* func, const char* file, int line)
{
	void* ptr = malloc(size);

	XFDBG("MALLOC[%p size %zu] - (%s):%s:%d",
	      ptr, size, func, file, line);
	return ptr;
}

void*
d_xcalloc(size_t count, size_t size, const char* func, const char* file, int line)
{
	void* ptr = calloc(count, size);

	XFDBG("CALLOC[%p count %zu, size %zu] - (%s):%s:%d",
	      ptr, count, size, func, file, line);
	return ptr;
}

void*
d_xrealloc(void* ptr, size_t size, const char* func, const char* file, int line)
{
	void* new_ptr = realloc(ptr, size);

	XFDBG("REALLOC[%p -> %p] (new size %zu) - (%s):%s:%d",
	      ptr, new_ptr, size, func, file, line);
	return new_ptr;
}

void
d_xfree(void* ptr, const char* func, const char* file, int line)
{
	XFDBG("FREE[%p] - (%s):%s:%d", ptr, func, file, line);
	free(ptr);
}
