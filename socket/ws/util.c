#include "util.h"

/*
 * host order is LE (unless we are working on a mainframe ;), network byte order of TCP BE
 * http://stackoverflow.com/questions/2100331/c-macro-definition-to-determine-big-endian-or-little-endian-machine
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

uint64_t
HeaderField_value(HeaderField field, void* data)
{
	uint8_t* start = ((uint8_t*)data) + field.offset;

	switch (field.type)
	{
	case HDR_FIELD_BOOL:
		return (*((uint8_t*)start) & field.bitmask) != 0;
	case HDR_FIELD_OCTET:
		return *((uint8_t*)start) & field.bitmask;
	case HDR_FIELD_INT16:
		return ntohs(*((uint16_t*)start)) & field.bitmask;
	case HDR_FIELD_INT32:
		return ntohs(*((uint32_t*)start)) & field.bitmask;
	case HDR_FIELD_INT64:
		return ntoh64(*((uint64_t*)start)) & field.bitmask;
	}
}

uint8_t
HeaderField_byte_value(HeaderField field, void* data)
{
	assert(field.type == HDR_FIELD_BOOL || field.type == HDR_FIELD_OCTET);
	return (uint8_t)(HeaderField_value(field, data));
}

uint16_t
HeaderField_uint16_value(HeaderField field, void* data)
{
	assert(field.type == HDR_FIELD_INT16);
	return (uint16_t)(HeaderField_value(field, data));
}

uint32_t
HeaderField_uint32_value(HeaderField field, void* data)
{
	assert(field.type == HDR_FIELD_INT32);
	return (uint32_t)(HeaderField_value(field, data));
}

uint64_t
HeaderField_uint64_value(HeaderField field, void* data)
{
	assert(field.type == HDR_FIELD_INT64);
	return (uint64_t)(HeaderField_value(field, data));
}

uint64_t
HeaderField_value_for(HeaderFieldType type, uint8_t offset, uint64_t bitmask, void* data)
{
	HeaderField field = { .type = type, .offset = offset, .bitmask = bitmask };

	return HeaderField_value(field, data);
}
