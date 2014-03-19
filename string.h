#ifndef _STRING_H
#define _STRING_H

#include "stddef.h"     /* size_t */
#include <stdlib.h>     /* free */
#include <string.h>     /* memcpy */
#include <unistd.h>     /* write */
#include "libcx-base/debug.h"
//#include "libcx-base/xmalloc.h"

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

#define SBuf_unused(buf) \
	(buf->length - buf->string->length)

#define S_last(s) \
	(s->length == 0) ? NULL : s->value[s->length - 1]

#define S_at(s, index) \
	s->value[((index >= 0) ? (long)index : index + (long)s->length)]

String*
String_init(const char *value, size_t length);

#define S_dup(value) \
	value == NULL ? NULL : String_init(value, strlen(value))

// grow or shrink the buffer
#define S_realloc(string, nchars) \
	realloc(string, sizeof(String) + sizeof(char) * (string->length + nchars));


StringBuffer*
StringBuffer_new(size_t length);

#define SBuf_free(buffer) \
	S_free(buffer->string); \
	free(buffer);

int
StringBuffer_make_room(StringBuffer *buffer, size_t nchars);

ssize_t
StringBuffer_ncopy(StringBuffer *buffer, ssize_t offset, const char* source, size_t nchars);

#define StringBuffer_ncat(buffer, offset, s) \
	StringBuffer_ncopy(buffer, offset, s->value, s->length);

#define StringBuffer_cat(buffer, s) \
	StringBuffer_ncopy(buffer, (ssize_t)buffer->string->length, s->value, s->length);

#define S_comp(a, b) \
	((a->length == b->length) ? strncmp(a->value, b->value, a->length) : ((long)a->length - (long)b->length))

ssize_t
StringBuffer_nread(StringBuffer *buffer, int offset, int fd, size_t nchars);

#define StringBuffer_fread(buffer, offset, file, nchars) \
	StringBuffer_nread(buffer, offset, fileno(file), nchars)

#define StringBuffer_read_append(buffer, fd, nchars) \
	StringBuffer_nread(buffer, S_last(buffer->string), fd, nchars)

#define StringBuffer_fread_append(buffer, file, nchars) \
	StringBuffer_nread(buffer, S_last(buffer->string), fileno(file), nchars)


#endif
