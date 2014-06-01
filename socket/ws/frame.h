#ifndef CX_WS_FRAME_H
#define CX_WS_FRAME_H

#include "websocket.h"
#include "util.h"

#define EXTENDED_PAYLOAD_LENGTH 126             /* 2 bytes (16 bit unsigned integer network byte order */
#define CONTINUED_EXTENDED_PAYLOAD_LENGTH 127   /* 8 bytes (64 bit unsigned integer (MSB 0)  */
#define EXTENDED_PAYLOAD_MAX 0xFFFF


size_t
WebsocketsFrame_get_payload_length(Websockets* ws);

#endif
