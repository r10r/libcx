#ifndef _ARRAY_H
#define _ARRAY_H

#include <stdlib.h>
#include "libcx-base/debug.h"

#ifndef ARRAY_GROW_LENGTH
#define ARRAY_GROW_LENGTH (16)
#endif

// delete is simply a[index] = NULL;
// access is simply a[index]
// shrink shifts elements to index 0 and removes NULL elements

/* multiple readers, one writer locking */
#ifdef _ARRAY_DISABLE_LOCKING
/* locking support is disabled */
#define _ARRAY_LOCK_READ(hdr)
#define _ARRAY_LOCK_WRITE(hdr)
#define _ARRAY_UNLOCK_READ(hdr)
#define _ARRAY_UNLOCK_WRITE(hdr)
#else
/* locking support is enabled */
#define _ARRAY_LOCK_READ(hdr) \
	if (hdr->f_lock) { XDBG("lock READ"); hdr->f_lock(hdr, 0); }
#define _ARRAY_LOCK_WRITE(hdr) \
	if (hdr->f_lock) { XDBG("lock WRITE"); hdr->f_lock(hdr, 1); }
#define _ARRAY_UNLOCK_READ(hdr) \
	if (hdr->f_unlock) { XDBG("unlock READ"); hdr->f_unlock(hdr, 0); }
#define _ARRAY_UNLOCK_WRITE(hdr) \
	if (hdr->f_unlock) { XDBG("unlock WRITE"); hdr->f_unlock(hdr, 1); }
#endif


/* simple dynamic array implementation */
typedef char** Array;
typedef void F_ArrayIterator (int index, void *data);
/* match element of array, similar to */
typedef int F_ArrayMatch (void *data, void *key);
struct array_header_t;

typedef void F_ArrayLock (struct array_header_t *hdr, int rw);

struct array_header_t
{
	unsigned int length;
	unsigned int unused;
#ifndef _ARRAY_DISABLE_LOCKING
	F_ArrayLock *f_lock;
	F_ArrayLock *f_unlock;
#endif
	char *buf[];
};

/* http://stackoverflow.com/questions/3922958/void-arithmetic */
/* http://stackoverflow.com/questions/3523145/pointer-arithmetic-for-void-pointer-in-c */
static inline struct array_header_t* Array_header(const Array a)
{
	size_t offset = sizeof(struct array_header_t) / sizeof(a);
	struct array_header_t *hdr = (struct array_header_t *)(a - offset);

	return hdr;
}

/* array data length */
static inline unsigned int Array_length(const Array a)
{
	return Array_header(a)->length;
}

/* available space */
static inline unsigned int Array_unused(const Array a)
{
	return Array_header(a)->unused;
}

/* next index */
static inline unsigned int Array_next(Array a)
{
	return Array_length(a) - Array_unused(a);
}

/* calculate required size of array (header + length) */
static inline size_t Array_size(int length)
{
	return sizeof(struct array_header_t) + sizeof(char *) * length;
}

/* get length of the given array */
static inline size_t Array_sizeof(Array a)
{
	return sizeof(struct array_header_t) + Array_header(a)->length * sizeof(char *);
}

/* last element of array */
static inline void* Array_last(Array a)
{
	return a[Array_next(a) - 1];
}

Array
Array_new(unsigned int length);

void
	Array_free(Array);

void
Array_each(Array a, F_ArrayIterator iterator);

/* @return the index of the matched element, or -1 */
int
Array_match(Array a, void *key, F_ArrayMatch f_array_match);

Array
Array_grow(Array a, unsigned int elements);

Array
Array_shrink(Array a);

Array
Array_push(Array a, void *data);

Array
Array_append(Array a, void *data);

void *
Array_pop(Array a);

//void *
//Array_shift(Array a);

//void
//Array_unshift(Array a);

//void
//Array_prepend(Array a);

#endif
