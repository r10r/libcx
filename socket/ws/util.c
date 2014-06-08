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

/* TODO make header fields bitmask typesafe */

uint64_t
HeaderField_value(HeaderField field, char* data)
{
	char* start = data + field.offset;

	switch (field.type)
	{
	case HDR_FIELD_BOOL:
		return (*((uint8_t*)start) & field.bitmask) != 0;
	case HDR_FIELD_OCTET:
		return *((uint8_t*)start) & field.bitmask;
	case HDR_FIELD_INT16:
		return ntohs(*((uint16_t*)start)) & field.bitmask;
	case HDR_FIELD_INT32:
		return ntohl(*((uint32_t*)start)) & field.bitmask;
	case HDR_FIELD_INT64:
		return ntoh64(*((uint64_t*)start)) & field.bitmask;
	}
#if defined(__GNUC__) && !defined(__clang__)

	/* dead code makes GCC happy,
	 * because it does not check that all cases are covered */
	assert(0);
	return 0;
#endif
}

uint8_t
HeaderField_byte_value(HeaderField field, char* data)
{
	assert(field.type == HDR_FIELD_BOOL || field.type == HDR_FIELD_OCTET);
	return (uint8_t)(HeaderField_value(field, data));
}

uint16_t
HeaderField_uint16_value(HeaderField field, char* data)
{
	assert(field.type == HDR_FIELD_INT16);
	return (uint16_t)(HeaderField_value(field, data));
}

uint32_t
HeaderField_uint32_value(HeaderField field, char* data)
{
	assert(field.type == HDR_FIELD_INT32);
	return (uint32_t)(HeaderField_value(field, data));
}

uint64_t
HeaderField_uint64_value(HeaderField field, char* data)
{
	assert(field.type == HDR_FIELD_INT64);
	return (uint64_t)(HeaderField_value(field, data));
}

uint64_t
HeaderField_value_for(HeaderFieldType type, uint8_t offset, uint64_t bitmask, char* data)
{
	HeaderField field = { .type = type, .offset = offset, .bitmask = bitmask };

	return HeaderField_value(field, data);
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
