#ifndef _CX_STRING_H
#define _CX_STRING_H

// TODO move StringBuffer into separate compilation unit

/*
 * strings should always be \0 terminated, so they be safely printed at any time
 * methods that append to a string buffer must overwrite the \0 terminator
 * methods that append to a string should be able to append to a string of zero length
 */

#include "stddef.h"     /* size_t */
#include <stdlib.h>     /* free */
#include <string.h>     /* memcpy */
#include <unistd.h>     /* write */

#include "base/debug.h"
//#include "base/xmalloc.h"

// 1 GiB = 2^30
#define STRING_MAX_LENGTH (1024 * 1024 * 1024)

typedef struct string_t
{
	size_t length;  /* used data */
	char value[];   /* incomplete type, can only be used as last field */
} String;

typedef struct string_pointer_t
{
	size_t length;
	const char* value;
} StringPointer;

// should the string contain data type information ?
typedef struct string_buffer_t
{
	size_t length;  /* total buffer length */
	String* string; /* we can now grow the string data */
} StringBuffer;

#define S_free(s) (free(s))

/* access array (with negative indexes), no bounds checking */
#define S_get(s, index) \
	((index < 0) \
	 ? ((s)->value + (s)->length - (size_t)-index) \
	 : ((s)->value + index))

#define S_last(s) \
	S_get(s, -1)

#define S_size(length) \
	(sizeof(String) + sizeof(char)* length)

#define S_dup(value) \
	String_init(value, strlen(value))

#define S_dupn(value) \
	String_ninit(value, strlen(value))

/* duplicate string and add null terminator */
#define S_ndupn(s) \
	String_ninit((s)->value, (s)->length)

#define S_alloc(length) \
	malloc(S_size(length))

// grow or shrink the buffer
#define S_realloc(s, length) \
	realloc(s, sizeof(String) + sizeof(char)* length);

#define S_comp(a, b) \
	(((a)->length == (b)->length) \
	 ? strncmp((a)->value, (b)->value, (a)->length) \
	 : ((long)(a)->length - (long)(b)->length))

/* compare string a with char* nc of length nc_len */
#define S_ncomp(a, nc) \
	(((a)->length == strlen(nc) \
	  ? strncmp((a)->value, nc, (a)->length) \
	  : ((long)(a)->length - strlen(nc)))

#define S_nwrite(s, start, count, fd) \
	write(fd, S_get(s, start), count)

#define S_fnwrite(s, start, count, stream) \
	S_nwrite(s, start, count, fileno(stream))

#define S_write(s, fd) \
	write(fd, (s)->value, (s)->length)

#define S_fwrite(s, stream) \
	S_write(s, fileno(stream))

/* write string + newline to file descriptor */
#define S_puts(s, fd) \
	(S_nwrite(s, 0, (s)->length, fd) + write(fd, "\n", sizeof(char)))

/* write string + newline to stream */
#define S_fputs(s, stream) \
	S_puts(s, fileno(stream))

#define S_ncopy(s, start, count, dst) \
	(strncpy(dst, S_get(s, start), count))

#define S_copy(s, dst) \
	S_ncopy(s, 0, (s)->length, dst)

String*
String_init(const char* value, size_t length);

String*
String_ninit(const char* value, size_t length);

int
String_shift(String* s, size_t count);


/* [ StringPointer methods ] */

StringPointer*
StringPointer_new(const char* data, size_t length);

void
StringPointer_free(StringPointer* pointer);

#endif
