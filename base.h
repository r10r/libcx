#ifndef _CX_BASE_H
#define _CX_BASE_H

#include <stddef.h>     /* offsetoff */
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
#define clone(type, obj) \
	(type*)memcpy(malloc(sizeof(type)), obj, sizeof(type))

#endif

