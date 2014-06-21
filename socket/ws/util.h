#ifndef _CX_WS_UTIL_H
#define _CX_WS_UTIL_H

#include <stdint.h>     /* uint64_t */
#include <arpa/inet.h>  /* ntohs ... */

#include "base/base.h"

typedef enum cx_processor_endian
{
	little_endian,
	big_endian
} ProcessorEndianType;

ProcessorEndianType
CheckCPUEndian(void);

/* host to network byte order */
uint64_t
hton64(uint64_t x);

uint64_t
ntoh64(uint64_t x);

/* TODO move to string_buffer.h (depends on arpa/inet.h) header */

#define StringBuffer_cat_htons(buf, num) \
	StringBuffer_cat_number(buf, htons(num), sizeof(uint16_t))

#define StringBuffer_cat_htonl(buf, num) \
	StringBuffer_cat_number(buf, htonl(num), sizeof(uint32_t))

#define StringBuffer_cat_hton64(buf, num) \
	StringBuffer_cat_number(buf, hton64(num), sizeof(uint64_t))

#endif
