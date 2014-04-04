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
	const char *value;
} StringPointer;


// should the string contain data type information ?
typedef struct string_buffer_t
{
	size_t length;                                          /* total buffer length */
	String *string;                                         /* we can now grow the string data */
} StringBuffer;

#define S_free(s) (free(s))

/* access array (with negative indexes), no bounds checking */
#define S_get(s, index) \
	((index < 0) ? (s->value + s->length - (size_t)-index) : (s->value + index))

#define S_last(s) \
	S_get(s, -1)

#define S_size(length) \
	(sizeof(String) + sizeof(char) * length)

#define S_dup(value) \
	String_init(value, strlen(value))

#define S_dupn(value) \
	String_ninit(value, strlen(value))

/* duplicate string and add null terminator */
#define S_ndupn(s) \
	String_ninit(s->value, s->length)

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
	write(fd, S_get(string, start), count)

#define S_fnwrite(string, start, count, stream) \
	S_nwrite(string, start, count, fileno(stream))

#define S_write(string, fd) \
	write(fd, string->value, string->length)

#define S_fwrite(string, stream) \
	S_write(string, fileno(stream))

/* write string + newline to file descriptor */
#define S_puts(string, fd) \
	(S_nwrite(string, 0, string->length, fd) + write(fd, "\n", sizeof(char)))

/* write string + newline to stream */
#define S_fputs(string, stream) \
	S_puts(string, fileno(stream))

#define S_ncopy(string, start, count, dst) \
	(strncpy(dst, S_get(string, start), count))

#define S_copy(string, dst) \
	S_ncopy(string, 0, string->length, dst)

String*
String_init(const char *value, size_t length);

String*
String_ninit(const char *value, size_t length);

int
String_shift(String *s, size_t count);

/* StringBuffer methods */

StringBuffer*
StringBuffer_new(size_t length);

void
StringBuffer_free(StringBuffer *buffer);

#define StringBuffer_unused(buf) \
	(buf->length - buf->string->length)

#define StringBuffer_used(buf) \
	(buf->string->length)

#define StringBuffer_value(buf) \
	(buf->string->value)

int
StringBuffer_make_room(StringBuffer *buffer, size_t offset, size_t nchars);

ssize_t
StringBuffer_append(StringBuffer *buffer, size_t offset, const char* source, size_t nchars);

#define StringBuffer_fload(buffer, file,  chunk_size) \
	StringBuffer_fdload(buffer, fileno(file), chunk_size)

ssize_t
StringBuffer_fdload(StringBuffer *buffer, int fd, size_t chunk_size);

#define StringBuffer_cat(buffer, chars) \
	StringBuffer_append(buffer, buffer->string->length, chars, strlen(chars))

#define StringBuffer_catn(buffer, chars) \
	StringBuffer_append(buffer, buffer->string->length, chars, strlen(chars) + 1)

#define StringBuffer_ncat(buffer, chars, nchars) \
	StringBuffer_append(buffer, buffer->string->length, chars, nchars)

#define StringBuffer_scat(buffer, s) \
	StringBuffer_append(buffer, buffer->string->length, s->value, s->length)

ssize_t
StringBuffer_read(StringBuffer *buffer, size_t offset, int fd, size_t nchars);

#define StringBuffer_fdcat(buffer, fd, nchars) \
	StringBuffer_read(buffer, buffer->string->length, fd, nchars)

#define StringBuffer_fcat(buffer, file, nchars) \
	StringBuffer_read(buffer, buffer->string->length, fileno(file), nchars)

#define StringBuffer_shift(buf, count) \
	String_shift(buf->string, count)

#define StringBuffer_clear(buf) \
	buf->string->length = 0;

/* StringPointer methods */
StringPointer*
StringPointer_new(const char* data, size_t length);

void
StringPointer_free(StringPointer *pointer);

#endif
