 /* do not replace malloc/realloc/free with macros here */
#define _XMALLOC_DISABLED
#include "xmalloc.h"

void *
d_xmalloc(size_t size, const char *func, const char *file, int line)
{
	void *ptr = malloc(size);
	printf("MALLOC[%p size %ld] - (%s):%s:%d\n",
			ptr, size, func, file, line);
	return ptr;
}

void *
d_xrealloc(void *ptr, size_t size, const char *func, const char *file, int line)
{
	void* new_ptr = realloc(ptr, size);
	printf("REALLOC[%p -> %p (new size %ld) - (%s):%s:%d\n",
			ptr, new_ptr, size, func, file, line);
	return new_ptr;
}

void
d_xfree(void *ptr, const char *func, const char *file, int line)
{
	printf("FREE[%p] - (%s):%s:%d\n", ptr, func, file, line);
	free(ptr);
}
