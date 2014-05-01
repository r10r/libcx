#ifndef CX_WS_H
#define CX_WS_H

#include <assert.h>
#include <stdint.h>     /* uint8_t */
#include <stdlib.h>     /* strtoul */
#include <netinet/in.h> /*htons*/
#include <string.h>
#include <stdio.h>      /* sscanf */
#include <ctype.h>      /* isdigit */
//#include <stddef.h> /* size_t macros (for printing) */
#include "base64_enc.h"
#include "sha1.h"

#include "umtp/message_parser.h"

typedef enum websockets_opcode_t
{
	WS_FRAME_CONTINUATION = 0x0,    /* a continuation frame */
	WS_FRAME_TEXT = 0x1,            /* a text frame */
	WS_FRAME_BINARY = 0x2,          /* a binary frame */
//	0x3 - 0x7 reserved for further non-control frames
	WS_FRAME_CLOSE = 0x8,           /* a connection close */
	WS_FRAME_PING = 0x9,            /* a ping */
	WS_FRAME_PONG = 0xA             /* a pong */
// 0xB - 0xF  reserved for further control frames
} WebsocketsOpcode;

typedef enum websockets_status_t
{
	WS_STATE_NEW,
	WS_STATE_ESTABLISHED,
	WS_STATE_CLOSED
} WebsocketsState;

typedef struct websockets_header_t
{
	unsigned int fin : 1;
	unsigned int rsv1 : 1;
	unsigned int rsv2 : 1;
	unsigned int rsv3 : 1;
	unsigned int opcode : 4;
	unsigned int mask : 1;
	unsigned int payload_length : 1;
} WebsocketsHeader;

typedef struct websockets_frame_t
{
	WebsocketsHeader header;
	WebsocketsOpcode opcode;        /* decoded opcode */
	uint64_t payload_length;        /* decoded payload length */
	uint32_t mask;                  /* contains the mask when masked */

	uint8_t* header_data;           /* pointer to the start of the frame */
	uint8_t* payload_data;          /* pointer to the start of the data (points to input buffer) */
} WebsocketsFrame;

typedef struct websockets_state_t
{
	WebsocketsFrame frame; /* the current incomming frame */
	WebsocketsState state;
	StringBuffer* in;
	StringBuffer* out;
} Websockets;

typedef struct websockets_handshake_t
{
	const char* host;
	const char* resource;

	const char* ws_key;
	const char* ws_protocol;        /* optional */
	const char* ws_extensions;      /* optional */

	const char* origin;             /* optional, required from browser */

	Message* message;               /* message where headers are mapped from */

	int error;
	StringBuffer* error_message;
} WebsocketsHandshake;

void
Websockets_parse_input_frame(Websockets* ws);

#endif  /* WEBSOCKET_H */
