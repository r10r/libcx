#include "array.h"
#include "libcx-base/debug.h"
/* NOTICE: list modification/access is not thread safe */

Array
Array_init(const void *value, unsigned long length)
{
	XASSERT("void pointer size should equal char pointer", sizeof(void *) == sizeof(char *));
	struct array_header_t *hdr;

	if (value)
		hdr =  malloc(Array_size(length));
	else
		hdr =  calloc(Array_size(length), 1);

//	XFDBG("header:%p\n", hdr);

	if (hdr == NULL)
		return NULL;         // OOM

	hdr->length = length;
	hdr->unused = length;

//	XFDBG("buf:%p\n", &hdr->buf);

	if (length > 0 && value)
		memcpy(hdr->buf, value, length);

#ifndef _ARRAY_DISABLE_LOCKING
	hdr->f_lock = NULL;
	hdr->f_unlock = NULL;
#endif

	return hdr->buf;
}

Array
Array_new(unsigned long length)
{
	return Array_init(NULL, length);
}

void
Array_free(Array a)
{
//	XFDBG("free:%p\n", a);
	if (a == NULL)
		return;
	free(Array_header(a));
}

void
Array_each(Array a, F_ArrayIterator f_array_iterator)
{
	struct array_header_t *hdr = Array_header(a);
	unsigned int i;

	_ARRAY_LOCK_READ(hdr);
	for (i = 0; i < hdr->length; i++)
		f_array_iterator(i, a[i]);
	_ARRAY_UNLOCK_READ(hdr);
}

int
Array_match(Array a, void *key, F_ArrayMatch f_array_match)
{
	int i_matched = -1;

	if (a == NULL)
		return i_matched;

	struct array_header_t *hdr = Array_header(a);
	unsigned int i;
	_ARRAY_LOCK_READ(hdr);
	for (i = 0; i < hdr->length; i++)
	{
		if (f_array_match(a[i], key) == 0)
		{
			i_matched = i;
			break;
		}
	}
	_ARRAY_UNLOCK_READ(hdr);

	return i_matched;
}

Array
Array_grow(Array a, unsigned long elements)
{
	struct array_header_t *hdr, *new_hdr;
	unsigned long unused = Array_unused(a);
	unsigned long length, new_length;

	if (unused > elements)
		return a;
	length = Array_length(a);
	hdr = Array_header(a);

	new_length = length + elements;

//  if (new_length < Array_MAX_PREALLOC)
//      new_length *= 2;
//  else
//      new_length += Array_MAX_PREALLOC;

	new_hdr = realloc(hdr, Array_size(new_length));
	if (new_hdr == NULL)
		return NULL;
	new_hdr->length = new_length;
	new_hdr->unused = new_length - length;

	return new_hdr->buf;
}

Array
Array_shrink(Array s)
{
	XASSERT("not yet implemented", 0);
	return NULL;
}

static inline Array
_Array_append(Array a, void *data)
{
	if (a == NULL)
		return NULL;
	if (data == NULL)
		return a;

	struct array_header_t *a_hdr = Array_header(a);

	_ARRAY_LOCK_WRITE(a_hdr);
	unsigned long next_index = Array_next(a);

	Array c = a;
	struct array_header_t *c_hdr = a_hdr;
	if (next_index == Array_length(a))
	{
		c = Array_grow(c, ARRAY_GROW_LENGTH);
		a_hdr = Array_header(c);
	}

	if (c == NULL)
		return NULL;        // OOM

	c[Array_next(c)] = data;
	a_hdr->unused--;
	_ARRAY_UNLOCK_WRITE(a_hdr);

	return c;
}

Array
Array_append(Array arr, void *data)
{
	return _Array_append(arr, data);
}

Array
Array_push(Array arr, void *data)
{
	return _Array_append(arr, data);
}

void *
Array_pop(Array arr)
{
	struct array_header_t *hdr = Array_header(arr);

	_ARRAY_LOCK_WRITE(hdr);
	unsigned long index = Array_next(arr);

	if (index == 0)
		return NULL;

	void *data = arr[Array_next(arr) - 1];
	arr[Array_next(arr) - 1] = NULL;
	hdr->unused++;
	_ARRAY_UNLOCK_WRITE(hdr);

	return data;
}
