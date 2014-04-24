#ifndef _CX_STRING_BUFFER_H
#define _CX_STRING_BUFFER_H

#include "string.h"
#include <stdarg.h>     /* vsnprintf, va_* */

StringBuffer*
StringBuffer_new(size_t length);

void
StringBuffer_init(StringBuffer* buffer, size_t length);

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

#define StringBuffer_value(buffer) \
	((buffer)->string->value)

#define StringBuffer_shift(buffer, count) \
	String_shift((buffer)->string, count)

#define StringBuffer_clear(buffer) \
	(buffer)->string->length = 0;


/* [ append from char* ] */

ssize_t
StringBuffer_append(StringBuffer* buffer, size_t offset, const char* source, size_t nchars);

#define StringBuffer_cat(buffer, chars) \
	StringBuffer_append(buffer, StringBuffer_used(buffer), chars, strlen(chars) + 1)

#define StringBuffer_ncat(buffer, chars, nchars) \
	StringBuffer_append(buffer, StringBuffer_used(buffer), chars, nchars)

#define StringBuffer_scat(buffer, s) \
	StringBuffer_append(buffer, StringBuffer_used(buffer), (s)->value, (s)->length)


/* [ append from FD or stream ] */

ssize_t
StringBuffer_read(StringBuffer* buffer, size_t offset, int fd, size_t nchars);

#define StringBuffer_fdcat(buffer, fd) \
	StringBuffer_read(buffer, StringBuffer_used(buffer), fd, StringBuffer_length(buffer))

#define StringBuffer_fcat(buffer, file) \
	StringBuffer_read(buffer, StringBuffer_used(buffer), fileno(file), StringBuffer_length(buffer))

#define StringBuffer_fdncat(buffer, fd, nchars) \
	StringBuffer_read(buffer, StringBuffer_used(buffer), fd, nchars)

#define StringBuffer_fncat(buffer, file, nchars) \
	StringBuffer_read(buffer, StringBuffer_used(buffer), fileno(file), nchars)

ssize_t
StringBuffer_fdload(StringBuffer* buffer, int fd, size_t chunk_size);

#define StringBuffer_fload(buffer, file,  chunk_size) \
	StringBuffer_fdload(buffer, fileno(file), chunk_size)


/* [ formatted printing ] */

/*
 * [LLVM Language Extensions](http://goo.gl/92gLsT)
 * [vsnprintf - Format String checking](http://goo.gl/aL1X57)
 */
__attribute__((__format__(__printf__, 3, 0)))
ssize_t
StringBuffer_vsprintf(StringBuffer* buffer, size_t offset, const char* format, ...);

#define StringBuffer_printf(buffer, format, ...) \
	StringBuffer_vsprintf(buffer, 0, format, __VA_ARGS__)

#define StringBuffer_aprintf(buffer, format, ...) \
	StringBuffer_vsprintf(buffer, StringBuffer_used(buffer) - 1, format, __VA_ARGS__)

#endif
