#ifndef _CX_WS_FRAME_H
#define _CX_WS_FRAME_H

#include "websocket.h"
#include "util.h"

#define WS_HEADER_SIZE 2
#define PAYLOAD_MAX 125

/* FIXME prefix with WS_, rename everything to size or byte */
#define PAYLOAD_EXTENDED 126             /* 16 bit unsigned integer (Big-Endian / MSB 0) */
#define PAYLOAD_EXTENDED_SIZE   2
#define PAYLOAD_EXTENDED_MAX 0xFFFF

#define PAYLOAD_EXTENDED_CONTINUED 127   /* 64 bit unsigned integer (Big-Endian / MSB 0)  */
#define PAYLOAD_EXTENDED_CONTINUED_SIZE 8

#define WS_MASKING_KEY_LENGTH 4

#define WS_FRAME_SIZE_MIN WS_HEADER_SIZE

#define WS_CONTROL_MESSAGE_PAYLOAD_SIZE_MAX PAYLOAD_MAX                                                                 /* 125 */
#define WS_CONTROL_MESSAGE_SIZE_MAX (WS_HEADER_SIZE + WS_CONTROL_MESSAGE_PAYLOAD_SIZE_MAX + WS_MASKING_KEY_LENGTH)      /* 131 */

/* error messages are control messages */
#define WS_STATUS_CODE_SIZE 2   /* 16 bit error code */
#define WS_STATUS_DATA_SIZE_MAX (WS_CONTROL_MESSAGE_PAYLOAD_SIZE_MAX - WS_STATUS_CODE_SIZE)

WebsocketsFrame*
WebsocketsFrame_dup(WebsocketsFrame* frame);

void
WebsocketsFrame_free(WebsocketsFrame* frame);

void
WebsocketsFrame_parse_header(WebsocketsFrame* frame, char* raw);

void
WebsocketsFrame_parse(WebsocketsFrame* frame, uint8_t* raw);

void
WebsocketsFrame_unmask_payload_data(WebsocketsFrame* frame);

void
WebsocketsFrame_parse_payload_length_extended(WebsocketsFrame* frame, char* raw);

StringBuffer*
WebsocketsFrame_write(uint8_t header_bits, const char* payload, uint64_t nchars, unsigned int masked);

#define WebsocketsFrame_create_message(opcode, payload, nchars) \
	WebsocketsFrame_create((uint8_t)WS_HDR_FIN.bitmask | opcode, payload, nchars)

/*
 * checks if frame is fully loaded into buffer, if not stop processing here
 * @return number of additional bytes (>= 0) or number of missing bytes (< 0)
 */
#define WebsocketsFrame_buffer_level(ws) \
	((int64_t)StringBuffer_used((ws)->in) - (int64_t)(ws)->frame.length)

StringBuffer*
WebsocketsFrame_create(uint8_t header_bits, const char* payload, uint64_t nchars);

StringBuffer*
WebsocketsFrame_create_error(WebsocketsStatusCode status_code, const char* message);

StringBuffer*
WebsocketsFrame_create_echo(WebsocketsFrame* frame);

uint16_t
WebsocketsFrame_response_status(WebsocketsFrame* frame);

#define WebsocketsFrame_log(frame) \
	XFDBG("\n" \
	      "--->	fin:%u, rsv1:%u, rsv2:%u, rsv3:%u, masked:%u, opcode:0x%x\n" \
	      "	payload_length:%u, payload_length_extended:%llu", \
	      frame->fin, frame->rsv1, frame->rsv2, frame->rsv3, frame->masked, frame->opcode, \
	      frame->payload_length, frame->payload_length_extended)

#endif

typedef enum cx_header_field_type_t
{
	HDR_FIELD_BOOL,         /* single bit */
	HDR_FIELD_OCTET,        /* 2 to 8 bit */
	HDR_FIELD_INT16,        /* 16 bit (converted with ntohs) */
	HDR_FIELD_INT32,        /* 32 bit (converted with ntohl) */
	HDR_FIELD_INT64         /* 64 bit (converted with ntohl) */
} HeaderFieldType;

typedef struct cx_header_field_t
{
	HeaderFieldType type;   /* field type */

	/* TODO use a union for the bitmask instead of casting ? */
	uint64_t bitmask;       /* bitmask to extract the value */
	uint8_t offset;         /* offset in bytes */
} HeaderField;

#define HDR_MASK_ALL    (~((uint64_t)0))
