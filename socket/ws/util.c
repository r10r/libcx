#include "util.h"

/*
 * host order is LE (unless we are working on a mainframe ;),
 * network byte order of TCP BE
 * see [stackoverflow - macro definition to detect endianess](http://goo.gl/8RTNkV)
 *
 */

uint64_t
hton64(uint64_t x)
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
ntoh64(uint64_t x)
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

ProcessorEndianType
CheckCPUEndian()
{
	unsigned short x;
	unsigned char c;
	ProcessorEndianType CPUEndian;

	x = 0x0001;
	;
	c = *(unsigned char*)(&x);
	if ( c == 0x01 )
		CPUEndian = little_endian;
	else
		CPUEndian = big_endian;

	return CPUEndian;
}
