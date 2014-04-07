#ifndef _CX_ARRAY_H
#define _CX_ARRAY_H

#include <stdlib.h>
#include "libcx-base/debug.h"

#ifndef ARRAY_GROW_LENGTH
#define ARRAY_GROW_LENGTH (16)
#endif

// delete is simply a[index] = NULL;
// access is simply a[index]
// shrink shifts elements to index 0 and removes NULL elements

/* simple dynamic array implementation */
typedef char** Array;
typedef void F_ArrayIterator (int index, void* data);
/* match element of array, similar to */
typedef int F_ArrayMatch (void* data, void* key);
struct array_header_t;

struct array_header_t
{
	unsigned long length;
	unsigned long unused;
	char* buf[];
};

/* http://stackoverflow.com/questions/3922958/void-arithmetic */
/* http://stackoverflow.com/questions/3523145/pointer-arithmetic-for-void-pointer-in-c */
static inline struct array_header_t*
Array_header(const Array a)
{
	size_t offset = sizeof(struct array_header_t) / sizeof(a);
	struct array_header_t* hdr = (struct array_header_t*)(a - offset);

	return hdr;
}

/* array data length */
static inline unsigned long
Array_length(const Array a)
{
	return Array_header(a)->length;
}

/* available space */
static inline unsigned long
Array_unused(const Array a)
{
	return Array_header(a)->unused;
}

/* next index */
static inline unsigned long
Array_next(Array a)
{
	return Array_length(a) - Array_unused(a);
}

/* calculate required size of array (header + length) */
static inline size_t
Array_size(unsigned long length)
{
	return sizeof(struct array_header_t) + sizeof(char*) * length;
}

/* get length of the given array */
static inline size_t
Array_sizeof(Array a)
{
	return sizeof(struct array_header_t) + Array_header(a)->length * sizeof(char*);
}

/* last element of array */
static inline void*
Array_last(Array a)
{
	return a[Array_next(a) - 1];
}

Array
Array_init(const void* value, unsigned long length);

Array
Array_new(unsigned long length);

void
	Array_free(Array);

void
Array_each(Array a, F_ArrayIterator iterator);

/* @return the index of the matched element, or -1 */
int
Array_match(Array a, void* key, F_ArrayMatch f_array_match);

Array
Array_grow(Array a, unsigned long elements);

Array
Array_shrink(Array a);

Array
Array_push(Array a, void* data);

Array
Array_append(Array a, void* data);

void*
Array_pop(Array a);

//void *
//Array_shift(Array a);

//void
//Array_unshift(Array a);

//void
//Array_prepend(Array a);

#endif
