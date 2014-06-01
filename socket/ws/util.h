#ifndef CX_WS_UTIL_H
#define CX_WS_UTIL_H

#include <stdint.h> /* uint64_t */

/* host to network byte order */
static uint64_t
htobe64(uint64_t x);

uint64_t
be64toh(uint64_t x);

#endif
