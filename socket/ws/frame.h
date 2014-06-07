#ifndef CX_WS_FRAME_H
#define CX_WS_FRAME_H

#include "websocket.h"
#include "util.h"

#define PAYLOAD_EXTENDED 126             /* 16 bit unsigned integer (Big-Endian / MSB 0) */
#define PAYLOAD_EXTENDED_SIZE   2
#define PAYLOAD_EXTENDED_MAX 0xFFFF

#define PAYLOAD_EXTENDED_CONTINUED 127   /* 64 bit unsigned integer (Big-Endian / MSB 0)  */
#define PAYLOAD_EXTENDED_CONTINUED_SIZE 8

#define WS_MASKING_KEY_LENGTH 4

void
WebsocketsFrame_parse_header(Websockets* ws);

/* -1 on error, 1 else */
int
WebsocketsFrame_process(Websockets* ws);

void
WebsocketsFrame_parse(Websockets* ws);

void
WebsocketsFrame_unmask_payload_data(Websockets* ws);

void
WebsocketsFrame_parse_payload_length_extended(Websockets* ws);

void
WebsocketsFrame_write_to_buffer(StringBuffer* buf, uint8_t header_bits, const char* payload, uint64_t nchars, unsigned int masked);

#define WebsocketsFrame_create(buf, opcode, payload, nchars) \
	WebsocketsFrame_write_to_buffer(buf, (uint8_t)WS_HDR_FIN.bitmask | opcode, payload, nchars, 0)

/*
 * checks if frame is fully loaded into buffer, if not stop processing here
 * @return number of additional bytes (>= 0) or number of missing bytes (< 0)
 */
#define WebsocketsFrame_buffer_level(ws) \
	(int64_t)(StringBuffer_used((ws)->in) - (ws)->frame.length)

#endif
