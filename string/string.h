#ifndef _CX_STRING_H
#define _CX_STRING_H

// TODO move StringBuffer into separate compilation unit

/*
 * strings should always be \0 terminated, so they be safely printed at any time
 * methods that append to a string buffer must overwrite the \0 terminator
 * methods that append to a string should be able to append to a string of zero length
 */

#include <stddef.h>     /* size_t */
#include <stdlib.h>     /* free */
#include <string.h>     /* memcpy */
#include <strings.h>    /* strcasecmp */
#include <unistd.h>     /* write */
#include <stdio.h>      /* fileno, vsnprintf */
#include <limits.h>     /* LONG_MAX */

#include "../base/base.h"

#ifndef STRING_MAX_LENGTH
// 1 GiB = 2^30
#define STRING_MAX_LENGTH SIZE_MAX
#endif

#define CX_ERR -1

typedef struct cx_string_t
{
	size_t length;  /* used data */
	char value[];   /* incomplete type, can only be used as last field */
} String;

typedef struct cx_string_pointer_t
{
	size_t length;
	const char* value;
} StringPointer;

String*
String_init(const char* value, size_t length);

int
String_shift(String* s, size_t count);

#define S_free(s) (cx_free(s))

/* TODO hide this macro from external usage
 */
#define __strpos(s, _pos) \
	(((_pos) < 0) ? ((s)->length - (size_t)-(_pos)) : (size_t)(_pos))

/* access array (with negative indexes), no bounds checking */
#define S_get(s, _pos) \
	((s)->value + __strpos(s, _pos))

/* access array (with negative indexes) with bounds checking */
#define S_sget(s, _pos) \
	((__strpos(s, _pos) >= (s)->length) ? NULL : S_get(s, __strpos(s, _pos)))

#define S_last(s) \
	((s)->length == 0 ? (s)->value : (s)->value + ((s)->length - 1))

/* return pointer to the last \0 terminator */
#define S_term(s) \
	((s)->length == 0 ? (s)->value : (s)->value + (s)->length)

/* set the last \0 terminator */
#define S_nullterm(s) \
	(*(S_term(s)) = '\0')

/* string size for given length including null terminator */
#define S_size(length) \
	(sizeof(String) + sizeof(char)*(length + 1 /* \0 */))

#define S_alloc(length) \
	cx_alloc(S_size(length))

#define S_dup(value) \
	String_init(value, strlen(value))

#define S_comp(a, b) \
	strcmp((a)->value, (b)->value)


/* [ write to FD/stream] */

#define S_nwrite(s, start, count, fd) \
	write(fd, S_get(s, start), count)

#define S_fnwrite(s, start, count, stream) \
	S_nwrite(s, start, count, fileno(stream))

#define S_write(s, fd) \
	S_nwrite(s, 0, (s)->length, fd)

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
	S_ncopy(s, 0, (s)->length + 1, dst)


/* [ StringPointer methods ] */

StringPointer*
StringPointer_new(const char* data, size_t length);

void
StringPointer_free(StringPointer* pointer);

#endif
