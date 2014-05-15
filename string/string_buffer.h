#ifndef _CX_STRING_BUFFER_H
#define _CX_STRING_BUFFER_H

#include "string.h"
#include <stdarg.h>     /* vsnprintf, va_* */
#include <stdint.h>     /* uint*_t */

/* READ_MAX must be <= SSIZE_MAX */
#define READ_MAX SSIZE_MAX

typedef enum cx_string_buffer_error_t
{
	STRING_ERROR_ERRNO = 1,
	STRING_ERROR_TO_SMALL,
	STRING_ERROR_INVALID_OFFSET
} StringBufferError;

// should the string contain data type information ?
typedef struct cx_string_buffer_t
{
	size_t length;                  /* total buffer length */
	String* string;                 /* we can now grow the string data */
	StringBufferError error;        /* error while processing input */
} StringBuffer;

StringBuffer*
StringBuffer_new(size_t length);

void
StringBuffer_init(StringBuffer* buffer, size_t length);

StringBuffer*
StringBuffer_from_string(String* string);

void
StringBuffer_free(StringBuffer* buffer);

void
StringBuffer_free_members(StringBuffer* buffer);

int
StringBuffer_make_room(StringBuffer* buffer, size_t offset, size_t nchars);

void
StringBuffer_set_error(StringBuffer* buffer, StringBufferError error);

#define StringBuffer_error(buffer) \
	((buffer)->error != 0)

/* [ utility macros ] */

#define StringBuffer_length(buffer) \
	(buffer)->length

#define StringBuffer_used(buffer) \
	((buffer)->string->length)

#define StringBuffer_unused(buffer) \
	(StringBuffer_length(buffer) - StringBuffer_used(buffer))

#define StringBuffer_index_append(buffer) \
	(StringBuffer_used(buffer) == 0 ? 0 : (StringBuffer_used(buffer)))

#define StringBuffer_value(buffer) \
	((buffer)->string->value)

#define StringBuffer_shift(buffer, count) \
	String_shift((buffer)->string, count)

#define StringBuffer_clear(buffer) \
	S_clear((buffer)->string)

#define StringBuffer_log(buf, message) \
	XFDBG("\n	%s [%p] - used:%zu, unused:%zu, length:%zu", \
	      message, (void*)buf, StringBuffer_used(buf), StringBuffer_unused(buf), StringBuffer_length(buf))

#define StringBuffer_equals(buf1, buf2) \
	(strcmp(StringBuffer_value(buf1), StringBuffer_value(buf2)) == 0)

#define StringBuffer_at(buf, index) \
	S_sget((buf)->string, index)

/* [ append from char* ] */

BufferStatus
StringBuffer_append(StringBuffer* buffer, size_t offset, const char* source, size_t nchars);

#define StringBuffer_cat(buffer, chars) \
	StringBuffer_append(buffer, StringBuffer_index_append(buffer), chars, strlen(chars))

#define StringBuffer_ncat(buffer, chars, nchars) \
	StringBuffer_append(buffer, StringBuffer_index_append(buffer), chars, nchars)

#define StringBuffer_scat(buffer, s) \
	StringBuffer_append(buffer, StringBuffer_index_append(buffer), (s)->value, (s)->length)


/* [ append from number ] */

BufferStatus
StringBuffer_append_number(StringBuffer* buffer, size_t offset, uint64_t num, size_t nbytes);

#define StringBuffer_cat_number(buffer, num, nbytes) \
	StringBuffer_append_number(buffer, StringBuffer_index_append(buffer), num, nbytes)


/* [ append from FD or stream ] */

/* single pass reading, maximum read length == SSIZE_MAX */

ssize_t
StringBuffer_read(StringBuffer* buffer, size_t offset, int fd, ssize_t nchars);

#define StringBuffer_fdcat(buffer, fd) \
	StringBuffer_read(buffer, StringBuffer_index_append(buffer), fd, StringBuffer_length(buffer))

#define StringBuffer_fcat(buffer, file) \
	StringBuffer_read(buffer, StringBuffer_index_append(buffer), fileno(file), StringBuffer_length(buffer))

#define StringBuffer_fdncat(buffer, fd, nchars) \
	StringBuffer_read(buffer, StringBuffer_index_append(buffer), fd, nchars)

#define StringBuffer_fncat(buffer, file, nchars) \
	StringBuffer_read(buffer, StringBuffer_index_append(buffer), fileno(file), nchars)


/* multi pass reading, maximum read length == SIZE_MAX */

BufferStatus
StringBuffer_fdxload(StringBuffer* buffer, int fd, size_t chunk_size, int block);

#define StringBuffer_fdload(buffer, fd, chunk_size) \
	StringBuffer_fdxload(buffer, fd, chunk_size, 1)

#define StringBuffer_fload(buffer, file,  chunk_size) \
	StringBuffer_fdload(buffer, fileno(file), chunk_size)

/* read until buffer is full */

BufferStatus
StringBuffer_ffill(StringBuffer* buffer, int fd, int block);


/* [ writing ] */

#define StringBuffer_write(buffer, fd) \
	write(fd, StringBuffer_value(buffer), StringBuffer_used(buffer));

#define StringBuffer_fwrite(buffer, stream) \
	StringBuffer_write(buffer, fileno(stream));

/* [ formatted printing ] */

/*
 * [LLVM Language Extensions](http://goo.gl/92gLsT)
 * [vsnprintf - Format String checking](http://goo.gl/aL1X57)
 */
__attribute__((__format__(__printf__, 3, 0)))
int
StringBuffer_vsnprintf(StringBuffer* buffer, size_t offset, const char* format, va_list args);

__attribute__((__format__(__printf__, 3, 0)))
int
StringBuffer_sprintf(StringBuffer* buffer, size_t offset, const char* format, ...);

__attribute__((__format__(__printf__, 2, 0)))
StringBuffer*
StringBuffer_from_printf(size_t length, const char* format, ...);


#define StringBuffer_printf(buffer, format, ...) \
	StringBuffer_sprintf(buffer, 0, format, __VA_ARGS__)

#define StringBuffer_aprintf(buffer, format, ...) \
	StringBuffer_sprintf(buffer, StringBuffer_index_append(buffer), format, __VA_ARGS__)


/* debug print n bytes fom bytes using format format into buf */
__attribute__((__format__(__printf__, 2, 0)))
void
StringBuffer_write_bytes_into(StringBuffer* buf, const char* const format, const uint8_t* bytes, size_t nbytes);

/* print hex values of bytes to stderr prepended by message */
void
StringBuffer_print_bytes_hex(StringBuffer* in, size_t nbytes, const char* message);

#endif
