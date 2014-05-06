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
#include "util.h"

#include "umtp/message_parser.h"

extern const HeaderField WS_HDR_FIN;
extern const HeaderField WS_HDR_RSV1;
extern const HeaderField WS_HDR_RSV2;
extern const HeaderField WS_HDR_RSV3;
extern const HeaderField WS_HDR_OPCODE;
extern const HeaderField WS_HDR_MASKED;
extern const HeaderField WS_HDR_PAYLOAD_LENGTH;
extern const HeaderField WS_HDR_PAYLOAD_LENGTH_EXT;
extern const HeaderField WS_HDR_PAYLOAD_LENGTH_EXT_CONTINUED;

/* https://tools.ietf.org/html/rfc6455#section-5.2 */
typedef enum websockets_opcode_t
{
	WS_FRAME_CONTINUATION = 0x0,    /* a continuation frame */
	WS_FRAME_TEXT = 0x1,            /* a text frame */
	WS_FRAME_BINARY = 0x2,          /* a binary frame */
//	0x3 - 0x7 reserved for further non-control frames
	WS_FRAME_CLOSE = 0x8,           /* a connection close */
	WS_FRAME_PING = 0x9,            /* a ping */
	WS_FRAME_PONG = 0xA,            /* a pong */
// 0xB - 0xF  reserved for further control frames
	WS_FRAME_UNDEFINED = 0xff       /* undefined frame opcode */
} WebsocketsOpcode;

/*
 * [Defined Status Codes](https://tools.ietf.org/html/rfc6455#section-7.4.1)
 *
 * [Reserved Status Code Ranges](see https://tools.ietf.org/html/rfc6455#section-7.4.2)
 *
 * - 0 - 999 unused
 * - 1000-2999 reserved for rfc6455
 * - 3000-3999 public use (libraries/frameworks/application) - IANA registration
 * - 4000-4999 private use
 */
typedef enum websockets_status_code_t
{
	WS_CODE_SUCCESS = 1000,                 /* normal closure */
	WS_CODE_MOVED = 1001,                   /* server going down / browser navigated away */
	WS_CODE_ERROR_PROTOCOL = 1002,          /* closure because of protocol error */
	WS_CODE_ERROR_MESSAGE_TYPE = 1003,      /* type of data can not be processed (e.g binary) */
	// 1004 reserved for future use
	// 1005 reserved for client (missing status code)
	// 1006 reserved for client (abnormal closed connection)
	WS_CODE_ERROR_MESSAGE_VALUE = 1007,     /* message contains an invalid token */
	WS_CODE_POLICY_VIOLATION = 1008,        /* generic policy violation */
	WS_CODE_POLICY_MESSAGE_TO_BIG = 1009,   /* message is to big to be processed */
	// 1010 reserved for client (missing extension)
	WS_CODE_SERVER_ERROR = 1011             /* unexpected server error */
	                                        // 1015 reserved for client (TLC handshake error)
} WebsocketsStatusCode;

typedef enum websockets_status_t
{
	WS_STATE_NEW,
	WS_STATE_ESTABLISHED,
	WS_STATE_CLOSED,
	WS_STATE_ERROR
} WebsocketsState;

typedef struct websockets_frame_t
{
	uint8_t* raw;
	uint8_t payload_offset;         /* payload offset from start of frame */
	WebsocketsOpcode opcode;        /* decoded opcode */

	uint64_t payload_length;        /* decoded payload length */
	uint8_t* payload_raw;           /* pointer to the start of the data (points to input buffer) */
	uint8_t* payload_raw_last;      /* pointer to last payload byte */
} WebsocketsFrame;

typedef struct websockets_state_t
{
	WebsocketsFrame frame; /* the current incomming frame */
	WebsocketsState state;
	StringBuffer* in;
	StringBuffer* out;
	StringBuffer* error_message;
} Websockets;

typedef struct websockets_handshake_t
{
	const char* host;               /* the server's authority */
	const char* resource;           /* the resource path from the request line */
	const char* ws_key;             /* a base64-encoded value (decoded 16 bytes) */
	const char* ws_protocol;        /* ordered (by preference) list of protocols  the client would like to speak */
	const char* ws_extensions;      /* list of extensions the client would like to speak */

	const char* origin;             /* optional (mandatory for browser clients) */

	Message* message;               /* message where headers are mapped from */

	int error;                      /* error code */
	StringBuffer* error_message;    /* error message explaining the error code */
} WebsocketsHandshake;

#endif  /* WEBSOCKET_H */
