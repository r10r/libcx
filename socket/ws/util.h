#ifndef CX_WS_UTIL_H
#define CX_WS_UTIL_H

#include <stdint.h>     /* uint64_t */
#include <arpa/inet.h>  /* ntohs ... */
#include <assert.h>

/* host to network byte order */
uint64_t
hton64(uint64_t x);

uint64_t
ntoh64(uint64_t x);

typedef enum header_field_type_t
{
	HDR_FIELD_BOOL,         /* single bit */
	HDR_FIELD_OCTET,        /* 2 to 8 bit */
	HDR_FIELD_INT16,        /* 16 bit (converted with ntohs) */
	HDR_FIELD_INT32,        /* 32 bit (converted with ntohl) */
	HDR_FIELD_INT64         /* 64 bit (converted with ntohl) */
} HeaderFieldType;

typedef struct header_field_t
{
	HeaderFieldType type;   /* field type */
	uint64_t bitmask;       /* bitmask to extract the value */
	uint8_t offset;         /* offset in bytes */
} HeaderField;

#define HDR_MASK_ALL    (~((uint64_t)0))

/* TODO inline these functions or convert to macros */

uint64_t
HeaderField_value(HeaderField field, void* data);

uint8_t
HeaderField_byte_value(HeaderField field, void* data);

uint16_t
HeaderField_uint16_value(HeaderField field, void* data);

uint32_t
HeaderField_uint32_value(HeaderField field, void* data);

uint64_t
HeaderField_uint64_value(HeaderField field, void* data);

uint64_t
HeaderField_value_for(HeaderFieldType type, uint8_t offset, uint64_t bitmask, void* data);

#endif