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

String*
String_init(const char* value, size_t length);

int
String_shift(String* s, size_t count);

#define S_free(s) (free(s))

#define strnlen(str) (strlen(str) + 1)

#define is_nullterm(source, nlength) \
	((source[nlength - 1] == '\0') ? 1 : 0)

/* access array (with negative indexes), no bounds checking */
#define S_get(s, index) \
	((index < 0) \
	 ? ((s)->value + (s)->length - (size_t)-index) \
	 : ((s)->value + index))

#define S_last(s) \
	((s)->length == 0 ? S_get(s, 0) : S_get(s, -1))

#define S_nullterm(s) \
	(* S_last(s) = '\0')

#define S_is_nullterm(s) \
	(* S_last(s) == '\0')

#define S_size(length) \
	(sizeof(String) + sizeof(char)* length)

#define S_dup(value) \
	String_init(value, strnlen(value))

#define S_alloc(length) \
	malloc(S_size(length))

// grow or shrink the buffer
#define S_realloc(s, length) \
	realloc(s, S_size(length));

#define S_comp(a, b) \
	strcmp((a)->value, (b)->value)

/* [ write to FD/stream] */

#define S_nwrite(s, start, count, fd) \
	write(fd, S_get(s, start), count)

#define S_fnwrite(s, start, count, stream) \
	S_nwrite(s, start, count, fileno(stream))

#define S_write(s, fd) \
	S_nwrite(s, 0, (s)->length - 1, fd)

#define S_fwrite(s, stream) \
	S_write(s, fileno(stream))

/* write string + newline to file descriptor */
#define S_puts(s, fd) \
	(S_write(s, fd) + write(fd, "\n", sizeof(char)))

/* write string + newline to stream */
#define S_fputs(s, stream) \
	S_puts(s, fileno(stream))

/* [ use memcpy instead ? ] */

#define S_ncopy(s, start, count, dst) \
	(strncpy(dst, S_get(s, start), count))

#define S_copy(s, dst) \
	S_ncopy(s, 0, (s)->length, dst)


/* [ StringPointer methods ] */

StringPointer*
StringPointer_new(const char* data, size_t length);

void
StringPointer_free(StringPointer* pointer);

#endif
