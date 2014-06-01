#ifndef CX_WS_UTIL_H
#define CX_WS_UTIL_H

#define EXTENDED_PAYLOAD_LENGTH 126             /* 2 bytes (16 bit unsigned integer network byte order */
#define CONTINUED_EXTENDED_PAYLOAD_LENGTH 127   /* 8 bytes (64 bit unsigned integer (MSB 0)  */
#define EXTENDED_PAYLOAD_MAX 0xFFFF

/* host to network byte order */
static uint64_t
htobe64(uint64_t x);

uint64_t
be64toh(uint64_t x);

#endif
