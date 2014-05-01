#include "util.h"

/*
 * host order is LE (unless we are working on a mainframe ;), network byte order of TCP BE
 * http://stackoverflow.com/questions/2100331/c-macro-definition-to-determine-big-endian-or-little-endian-machine
 *
 */

uint64_t
htobe64(uint64_t x)
{
	return (x >> 56) |
	       ((x << 40) & 0x00ff000000000000LL) |
	       ((x << 24) & 0x0000ff0000000000LL) |
	       ((x << 8) & 0x000000ff00000000LL) |
	       ((x >> 8) & 0x00000000ff000000LL) |
	       ((x >> 24) & 0x0000000000ff0000LL) |
	       ((x >> 40) & 0x000000000000ff00LL) |
	       (x << 56);
}

uint64_t
be64toh(uint64_t x)
{
	return (x >> 56) |
	       ((x << 40) & 0x00ff000000000000LL) |
	       ((x << 24) & 0x0000ff0000000000LL) |
	       ((x << 8) & 0x000000ff00000000LL) |
	       ((x >> 8) & 0x00000000ff000000LL) |
	       ((x >> 24) & 0x0000000000ff0000LL) |
	       ((x >> 40) & 0x000000000000ff00LL) |
	       (x << 56);
}
