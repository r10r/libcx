#ifndef _CX_STRING_H
#define _CX_STRING_H

#include "stddef.h"     /* size_t */
#include <stdlib.h>     /* free */
#include <string.h>     /* memcpy */
#include <unistd.h>     /* write */
#include "libcx-base/debug.h"
//#include "libcx-base/xmalloc.h"

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
	const char *data;
} StringPointer;


// should the string contain data type information ?
typedef struct string_buffer_t
{
	size_t length;                                          /* total buffer length */
	String *string;                                         /* we can now grow the string data */
} StringBuffer;

#define S_free(s) (free(s))

// FIXME doesn't work when string length = 0 !!!
/* access array (with negative indexes), no bounds checking */
#define S_get(s, index) \
	(s->value[((index >= 0) ? (long)index : (long)index + (long)s->length)])

#define S_last(s) \
	S_get(s, -1)

#define S_size(length) \
	(sizeof(String) + sizeof(char) * length)

#define S_dup(value) \
	value == NULL ? NULL : String_init(value, strlen(value))

#define S_dupn(value) \
	value == NULL ? NULL : String_ninit(value, strlen(value))

/* duplicate string and add null terminator */
#define S_ndupn(s) \
	s == NULL ? NULL : String_ninit(s->value, s->length)

#define S_alloc(length) \
	malloc(S_size(length))

// grow or shrink the buffer
#define S_realloc(string, length) \
	realloc(string, sizeof(String) + sizeof(char) * length);

#define S_comp(a, b) \
	((a->length == b->length) ? strncmp(a->value, b->value, a->length) : ((long)a->length - (long)b->length))

/* compare string a with char* nc of length nc_len */
#define S_ncomp(a, nc) \
	((a->length == strlen(nc) ? strncmp(a->value, nc, a->length) : ((long)a->length - strlen(nc)))

#define S_nwrite(string, start, count, fd) \
	write(fd, &string->value[start], count)

#define S_fnwrite(string, start, count, stream) \
	S_nwrite(string, start, count, fileno(stream))

#define S_write(string, fd) \
	write(fd, &string->value[0], string->length)

#define S_fwrite(string, stream) \
	S_write(string, fileno(stream))

/* write string + newline to file descriptor */
#define S_puts(string, fd) \
	(S_nwrite(string, 0, string->length, fd) + write(fd, "\n", sizeof(char)))

/* write string + newline to stream */
#define S_fputs(string, stream) \
	S_puts(string, fileno(stream))

#define S_ncopy(string, start, count, dst) \
	(strncpy(dst, &string->value[start], count))

#define S_copy(string, dst) \
	S_ncopy(string, 0, string->length, dst)

String*
String_init(const char *value, size_t length);

String*
String_ninit(const char *value, size_t length);

StringBuffer*
StringBuffer_new(size_t length);

void
StringBuffer_free(StringBuffer *buffer);

#define SBuf_unused(buf) \
	(buf->length - buf->string->length)

int
StringBuffer_make_room(StringBuffer *buffer, size_t index, size_t nchars);

ssize_t
StringBuffer_ncopy(StringBuffer *buffer, size_t index, const char* source, size_t nchars);

#define StringBuffer_nappend(buffer, chars, len) \
	StringBuffer_ncopy(buffer, buffer->string->length, chars, len)

#define StringBuffer_ncat(buffer, offset, s) \
	StringBuffer_ncopy(buffer, offset, s->value, s->length)

#define StringBuffer_cat(buffer, s) \
	StringBuffer_ncopy(buffer, buffer->string->length, s->value, s->length)

ssize_t
StringBuffer_nread(StringBuffer *buffer, size_t index, int fd, size_t nchars);

#define StringBuffer_fread(buffer, index, file, nchars) \
	StringBuffer_nread(buffer, index, fileno(file), nchars)

#define StringBuffer_read_append(buffer, fd, nchars) \
	StringBuffer_nread(buffer, buffer->string->length, fd, nchars)

#define StringBuffer_fread_append(buffer, file, nchars) \
	StringBuffer_nread(buffer, buffer->string->length, fileno(file), nchars)

#endif
