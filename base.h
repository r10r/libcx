#ifndef _BASE_H
#define _BASE_H

#include <stddef.h>     /* offsetoff */
/* see http://www.kroah.com/log/linux/container_of.html */
/* http://psomas.wordpress.com/2009/07/01/weird-kernel-macros-container_of/ */

/*
 * Typechecking ANSI-C conformable version of container_of
 * @from http://stackoverflow.com/questions/10269685/kernels-container-of-any-way-to-make-it-iso-conforming
 *
 * @ptr pointer to the member
 * @type type of the container
 * @member name of the member in the container
 *
 * -Werror=compare-distinct-pointer-types
 */
#define container_of(ptr, type, member) \
	((type*)((char*)(ptr) - offsetof(type, member) + \
		 (&((type*)0)->member == (ptr)) * 0))

/*
 * unchecked version of the above
 */
#define UC_container_of(ptr, type, member)      \
	(type*)( (char*)(ptr) - offsetof(type, member))

#endif

