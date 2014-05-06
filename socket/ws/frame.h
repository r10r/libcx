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

/* -1 on error, 1 else */
int
WebsocketsFrame_parse(Websockets* ws);

void
WebsocketsFrame_unmask_payload_data(Websockets* ws);

void
WebsocketsFrame_parse_payload_length_extended(Websockets* ws);

void
WebsocketsFrame_create_data_frame(StringBuffer* buf, const char* payload, uint64_t nchars, WebsocketsOpcode opcode);

#endif
