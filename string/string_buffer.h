#ifndef _CX_STRING_BUFFER_H
#define _CX_STRING_BUFFER_H

#include "string.h"
#include <stdarg.h>     /* vsnprintf, va_* */
#include <stdint.h>     /* uint*_t */
#include <stdbool.h>    /* bool */

/* READ_SIZE_MAX must be <= SSIZE_MAX */
#define READ_SIZE_MAX SSIZE_MAX

typedef enum cx_string_buffer_status_t
{
	STRING_BUFFER_STATUS_OK = 0,
	STRING_BUFFER_STATUS_EOF,
	STRING_BUFFER_STATUS_ERROR_ERRNO,
	STRING_BUFFER_STATUS_ERROR_TO_SMALL,
	STRING_BUFFER_STATUS_ERROR_INVALID_ACCESS,
	STRING_BUFFER_STATUS_ERROR_INVALID_READ_SIZE
} StringBufferStatus;

typedef struct cx_string_buffer_t
{
	size_t length;                  /* total buffer length */
	String* string;                 /* we can now grow the string data */
	StringBufferStatus status;      /* input processing status */
	int error_errno;                /* when status is error is due to errno */
} StringBuffer;

#define STRING_BUFFER_FERROR(buffer, _error_, _format_, ...) \
	XFERR("Buffer[%p l:%zu u:%zu] - error[%d:%s] " _format_, \
	      (void*)buffer, StringBuffer_length(buffer), StringBuffer_used(buffer), \
	      _error_, cx_strstatus(_error_), __VA_ARGS__); \
	(buffer)->status = _error_

#define STRING_BUFFER_ERROR(buffer, _error_, _message_) \
	STRING_BUFFER_FERROR(buffer, _error_, "%s", _message_)

#define STRING_BUFFER_ERRNO(buffer) \
	STRING_BUFFER_ERROR(buffer, STRING_BUFFER_STATUS_ERROR_ERRNO, strerror(errno)); \
	(buffer)->error_errno = errno

#define STRING_BUFFER_FDBG(buf, _format_, ...) \
	XFDBG("Buffer[%p l:%zu u:%zu] - " _format_, \
	      (void*)buf,  StringBuffer_length(buf), StringBuffer_used(buf), __VA_ARGS__)

#define STRING_BUFFER_DBG(buf, _message_) \
	STRING_BUFFER_FDBG(buf, "%s", _message_)

#define STRING_BUFFER_FWARN(buf, _format_, ...) \
	XFWARN("Buffer[%p l:%zu u:%zu] - " _format_, \
	       (void*)buf,  StringBuffer_length(buf), StringBuffer_used(buf), __VA_ARGS__)

#define STRING_BUFFER_WARN(buf, _message_) \
	STRING_BUFFER_FWARN(buf, "%s", _message_)

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

bool
StringBuffer_make_room(StringBuffer* buffer, size_t offset, size_t nchars);

void
StringBuffer_shift(StringBuffer* buffer, size_t count);

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

#define StringBuffer_clear(buffer) \
	StringBuffer_shift(buffer, StringBuffer_used(buffer))

#define StringBuffer_equals(buf1, buf2) \
	(strcmp(StringBuffer_value(buf1), StringBuffer_value(buf2)) == 0)

#define StringBuffer_at(buf, index) \
	S_sget((buf)->string, index)

/* [ append from char* ] */

void
StringBuffer_append(StringBuffer* buffer, size_t offset, const char* source, size_t nchars);

#define StringBuffer_cat(buffer, chars) \
	StringBuffer_append(buffer, StringBuffer_index_append(buffer), chars, strlen(chars))

#define StringBuffer_ncat(buffer, chars, nchars) \
	StringBuffer_append(buffer, StringBuffer_index_append(buffer), chars, nchars)

#define StringBuffer_scat(buffer, s) \
	StringBuffer_append(buffer, StringBuffer_index_append(buffer), (s)->value, (s)->length)


/* [ append from number ] */

void
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


/* [multi pass reading] */

/* read until there is no more input */
void
StringBuffer_fdload(StringBuffer* buffer, int fd, size_t read_size);

#define StringBuffer_fload(buffer, file, read_size) \
	StringBuffer_fdload(buffer, fileno(file), read_size)

/* read until buffer is full */
void
StringBuffer_ffill(StringBuffer* buffer, int fd);

#define StringBuffer_fill(buffer, file) \
	StringBuffer_ffill(buffer, fileno(file))


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

const char*
cx_strstatus(StringBufferStatus status);

#endif
