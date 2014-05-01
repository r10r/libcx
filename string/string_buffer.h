#ifndef _CX_STRING_BUFFER_H
#define _CX_STRING_BUFFER_H

#include "string.h"
#include <stdarg.h>     /* vsnprintf, va_* */

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


/* [ utility macros ] */

#define StringBuffer_length(buffer) \
	(buffer)->length

#define StringBuffer_used(buffer) \
	((buffer)->string->length)

#define StringBuffer_unused(buffer) \
	(StringBuffer_length(buffer) - StringBuffer_used(buffer))

#define StringBuffer_index_append(buffer) \
	StringBuffer_used(buffer) == 0 ? 0 : (StringBuffer_used(buffer) - 1)

#define StringBuffer_value(buffer) \
	((buffer)->string->value)

#define StringBuffer_shift(buffer, count) \
	String_shift((buffer)->string, count)

#define StringBuffer_clear(buffer) \
	(buffer)->string->length = 0; \
	S_nullterm((buffer)->string)

#define StringBuffer_log(buf, message) \
	XFDBG("\n	%s [%p] - used:%zu, unused:%zu, length:%zu", \
	      message, buf, StringBuffer_used(buf), StringBuffer_unused(buf), StringBuffer_length(buf))

/* [ append from char* ] */

ssize_t
StringBuffer_append(StringBuffer* buffer, size_t offset, const char* source, size_t nchars);

#define StringBuffer_cat(buffer, chars) \
	StringBuffer_append(buffer, StringBuffer_index_append(buffer), chars, strnlen(chars))

#define StringBuffer_ncat(buffer, chars, nchars) \
	StringBuffer_append(buffer, StringBuffer_index_append(buffer), chars, nchars)

#define StringBuffer_scat(buffer, s) \
	StringBuffer_append(buffer, StringBuffer_index_append(buffer), (s)->value, (s)->length)


/* [ append from FD or stream ] */

ssize_t
StringBuffer_read(StringBuffer* buffer, size_t offset, int fd, size_t nchars);

#define StringBuffer_fdcat(buffer, fd) \
	StringBuffer_read(buffer, StringBuffer_index_append(buffer), fd, StringBuffer_length(buffer))

#define StringBuffer_fcat(buffer, file) \
	StringBuffer_read(buffer, StringBuffer_index_append(buffer), fileno(file), StringBuffer_length(buffer))

#define StringBuffer_fdncat(buffer, fd, nchars) \
	StringBuffer_read(buffer, StringBuffer_index_append(buffer), fd, nchars)

#define StringBuffer_fncat(buffer, file, nchars) \
	StringBuffer_read(buffer, StringBuffer_index_append(buffer), fileno(file), nchars)

#define StringBuffer_fdload(buffer, fd, chunk_size) \
	StringBuffer_fdxload(buffer, fd, chunk_size, 1)

ssize_t
StringBuffer_fdxload(StringBuffer* buffer, int fd, size_t chunk_size, int blocking);

#define StringBuffer_fload(buffer, file,  chunk_size) \
	StringBuffer_fdload(buffer, fileno(file), chunk_size)


/* [ formatted printing ] */

/*
 * [LLVM Language Extensions](http://goo.gl/92gLsT)
 * [vsnprintf - Format String checking](http://goo.gl/aL1X57)
 */
__attribute__((__format__(__printf__, 3, 0)))
ssize_t
StringBuffer_vsnprintf(StringBuffer* buffer, size_t offset, const char* format, va_list args);

__attribute__((__format__(__printf__, 3, 0)))
ssize_t
StringBuffer_sprintf(StringBuffer* buffer, size_t offset, const char* format, ...);

__attribute__((__format__(__printf__, 2, 0)))
StringBuffer*
StringBuffer_from_printf(size_t length, const char* format, ...);


#define StringBuffer_printf(buffer, format, ...) \
	StringBuffer_sprintf(buffer, 0, format, __VA_ARGS__)

#define StringBuffer_aprintf(buffer, format, ...) \
	StringBuffer_sprintf(buffer, StringBuffer_index_append(buffer), format, __VA_ARGS__)

#endif
