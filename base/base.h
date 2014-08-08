#ifndef _CX_BASE_H
#define _CX_BASE_H

#include <stddef.h>     /* offsetoff */
#include <limits.h>

#include "debug.h"
#include "assert.h"
#include "mem.h"

/* Marker for functions that return malloced data that has to be freed afterwards */
#define CX_ALLOC

/* Macro to mark variables that cannot be made static as hidden */
#define CX_HIDDEN(var) ___cx_ ## var ## ___

#ifndef SSIZE_MAX
#define SSIZE_MAX LONG_MAX
//typedef unsigned long ssize_t;
#endif

#define THOUSAND 1000
#define MILLION  (THOUSAND * THOUSAND)
#define BILLION  (THOUSAND * THOUSAND * THOUSAND)

#define TIMEVAL_SET_MILLIS(_timeval, millis) \
	_timeval.tv_sec = (millis / THOUSAND); \
	_timeval.tv_usec = (millis % THOUSAND) * THOUSAND

/* [ memory management ] */


/* see http://www.kroah.com/log/linux/container_of.html */
/* http://psomas.wordpress.com/2009/07/01/weird-kernel-macros-container_of/ */

/*
 * Type safe ANSI-C conformable version of container_of
 * @from http://stackoverflow.com/questions/10269685/kernels-container-of-any-way-to-make-it-iso-conforming
 *
 * @ptr pointer to the member
 * @type type of the container
 * @member name of the member in the container
 *
 * requires clang with `-Werror=compare-distinct-pointer-types` enabled
 */
#define container_of(ptr, type, member) \
	((type*)((char*)(ptr) - offsetof(type, member) + \
		 (&((type*)0)->member == (ptr)) * 0))

/*
 * unchecked version of the above
 */
#define UC_container_of(ptr, type, member)      \
	(type*)( (char*)(ptr) - offsetof(type, member))

/* flat object cloning */
/* WARNING clone macro is defined in pthread.h (under linux) */
#define CX_clone(type, obj) \
	(type*)memcpy(cx_alloc(sizeof(type)), obj, sizeof(type))

#define EACH(iter, elem, next) \
	for (elem = NULL; iter && (elem = iter); iter = (iter)->next)

#define EACH_WITH_INDEX(iter, elem, next, index) \
	for (elem = NULL; iter && (elem = iter); iter = (iter)->next, index++)

/* for safely printing NULL values */
#define NULLS(val) \
	(val ? val : "(null)")

/* requires sys/time header */
#define timeval_diff(t_start, t_end) \
	((double)(((t_end)->tv_sec - (t_start)->tv_sec) * 1000.0 + ((t_end)->tv_usec - (t_start)->tv_usec) / 1000.0))

/* [ bit operations ]
 *
 * - http://frank.harvard.edu/~coldwell/toolchain/bigendian.html
 * - http://stackoverflow.com/questions/13514614/why-is-network-byte-order-defined-to-be-big-endian
 * - http://www.microhowto.info/howto/send_an_arbitrary_ethernet_frame_using_an_af_packet_socket_in_c.html
 * - http://www.linuxjournal.com/article/6788
 * - http://betterexplained.com/articles/understanding-big-and-little-endian-byte-order/
 */

/*
 * use bit fields (man htons)
 *
 */

/* [ litte endian ] */

#define LE_BIT(n) \
	(1 << n)

/* macro to set bit "n" in byte "mask" */
#define LE_BIT_SET(mask, n) \
	((mask) | (1 << n))

/* macro to clear bit "n" in byte "mask" */
#define LE_BIT_CLEAR(mask, n) \
	((mask) & ~(1 << n))

/* macro to return the value of bit "n" in byte "mask" */
#define LE_BIT_GET(mask, n) \
	(((mask) & (1 << n)) != 0)


/* [ big endian (byte with word length 8) ] */

#define BE_BIT(n) \
	(0x80 >> n)

/* macro to set bit "n" in byte "mask" */
#define BE_BIT_SET(mask, n) \
	((mask) | (0x80 >> n))

/* macro to clear bit "n" in byte "mask" */
#define BE_BIT_CLEAR(mask, n) \
	((mask) & ~(0x80 >> n))

/* macro to return the value of bit "n" in byte "mask" */
#define BE_BIT_GET(mask, n) \
	(((mask) & (0x80 >> n)) != 0)


#define BIT LE_BIT
#define BIT_SET LE_BIT_SET
#define BIT_GET LE_BIG_GET
#define BIT_CLEAR LE_BIT_CLEAR


/* assertions for safe type narrowing */
#define ASSERT_LONG(val) assert((long long val) < LONG_MAX && (long long)val > LONG_MIN)

#define UNUSED(x) (void)(x)

#endif
