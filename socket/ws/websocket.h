#ifndef _CX_WS_H
#define _CX_WS_H

#include <stdint.h>     /* uint8_t */
#include <stdlib.h>     /* strtoul */
#include <netinet/in.h> /*htons*/
#include <string.h>
#include <stdio.h>      /* sscanf */
#include <ctype.h>      /* isdigit */
//#include <stddef.h> /* size_t macros (for printing) */
#include <inttypes.h>   /* print types uint64_t */

#include <libcx/string/string_buffer.h>
#include <libcx/umtp/message_parser.h>

#include "base64_enc.h"
#include "sha1.h"
#include "util.h"

#define WS_OK 1
#define WS_ERR 0
#define FRAME_HEX_NPRINT 16     /* number of bytes to print for debugging */

//#define WS_BUFFER_SIZE WS_CONTROL_MESSAGE_SIZE_MAX
#define WS_BUFFER_SIZE 1024 * 1024 * 4 /* 4miB */

#define WS_HANDSHAKE_BUFFER_SIZE 512

#define WS_FRAME_LENGTH_MIN 2

/* https://tools.ietf.org/html/rfc6455#section-5.2 */
typedef enum cx_websockets_opcode_t
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
 */
#define WS_CODE_PROTOCOL_MIN 1000       /* reserved for rfc6455 */
#define WS_CODE_PROTOCOL_MAX 2999
#define WS_CODE_PUBLIC_MIN 3000         /* public use (libraries/frameworks/application) - IANA registration required */
#define WS_CODE_PUBLIC_MAX 3999
#define WS_CODE_PRIVATE_MIN 4000        /* private use */
#define WS_CODE_PRIVATE_MAX 4999

typedef enum cx_websockets_status_code_t
{
	WS_CODE_SUCCESS = 1000,                         /* normal closure */
	WS_CODE_MOVED = 1001,                           /* server going down / browser navigated away */
	WS_CODE_ERROR_PROTOCOL = 1002,                  /* closure because of protocol error */
	WS_CODE_ERROR_MESSAGE_TYPE = 1003,              /* type of data can not be processed (e.g binary) */
	// 1004 reserved for future use
	WS_CODE_CLIENT_MISSING_STATUS_CODE = 1005,      /*  1005 reserved for client (missing status code) */
	WS_CODE_ClIENT_ERROR = 1006,                    /* reserved for client (abnormal closed connection) */
	WS_CODE_ERROR_MESSAGE_VALUE = 1007,             /* message contains an invalid token */
	WS_CODE_POLICY_VIOLATION = 1008,                /* generic policy violation (TODO check only for server ?) */
	WS_CODE_POLICY_MESSAGE_TO_BIG = 1009,           /* message is to big to be processed (TODO check only for server ?) */
	WS_CODE_CLIENT_MISSING_EXTENSION = 1010,        /* reserved for client (missing extension) */
	WS_CODE_SERVER_ERROR = 1011,                    /* unexpected server error */
	WS_CODE_CLIENT_TLS_HANDSHAKE_ERROR = 1015       /* reserved for client (TLS handshake error) */
} WebsocketsStatusCode;

typedef enum cx_websockets_status_t
{
	WS_STATE_NEW,
	WS_STATE_ESTABLISHED,
	WS_STATE_CLOSE,
	WS_STATE_FRAME_MISSING_LENGTH,
	WS_STATE_FRAME_INCOMPLETE,
	WS_STATE_ERROR_HANDSHAKE_FAILED,
	WS_STATE_ERROR, /* generic error ? */
} WebsocketsState;

typedef struct cx_websockets_frame_t WebsocketsFrame;

struct cx_websockets_frame_t
{
	/* pointers set by WebsocketsFrame_parse */
	uint8_t* raw;
	uint8_t* payload_raw;                   /* pointer to the start of the data (points to input buffer) */
	uint8_t* payload_raw_end;               /* pointer to last payload byte */
	uint8_t* masking_key;                   /* pointer to the masking key */
	uint64_t payload_length_extended;       /* decoded payload length */
	uint64_t length;                        /* payload_length_extended + payload_offset */

	uint8_t payload_offset;                 /* payload offset from start of frame */
	uint8_t opcode;
	uint8_t payload_length;

	/* decoded bit fields */
	unsigned int fin : 1;
	unsigned int rsv1 : 1;
	unsigned int rsv2 : 1;
	unsigned int rsv3 : 1;
	unsigned int masked : 1;        /* bit field whether frame is masked or not */

	WebsocketsFrame* next;
	uint8_t* data;                 /* duplicated data */
};

typedef struct cx_websockets_state_t
{
	WebsocketsFrame frame; /* the current incomming frame */
	WebsocketsState state;
	StringBuffer* in;

	WebsocketsStatusCode status_code;
	StringBuffer* error_message;

	StringBuffer* fragments_buffer;
	uint8_t fragments_opcode;

	unsigned num_pings_without_pong;
} Websockets;

typedef struct cx_websockets_handshake_t
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
