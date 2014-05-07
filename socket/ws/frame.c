#include "frame.h"

void
WebsocketsFrame_unmask_payload_data(Websockets* ws)
{
	XDBG("Unmasking data");
	size_t i = 0;
	uint8_t* payload_byte = ws->frame.payload_raw;

	for (i = 0; i < ws->frame.payload_length_extended; i++)
	{
		/* we are corrupting memory */
		assert(payload_byte < ws->frame.payload_raw_end);

		uint8_t key = *(ws->frame.masking_key + (i % WS_MASKING_KEY_LENGTH));
		*payload_byte = *payload_byte ^ key;
		payload_byte++;
	}
}

void
WebsocketsFrame_parse_payload_length_extended(Websockets* ws)
{
	uint8_t* raw = (uint8_t*)StringBuffer_value(ws->in);

	switch (ws->frame.payload_length)
	{
	case PAYLOAD_EXTENDED:
		ws->frame.payload_offset += PAYLOAD_EXTENDED_SIZE;
		assert(StringBuffer_used(ws->in) > ws->frame.payload_offset); /* avoid memory corruption */
		ws->frame.payload_length_extended = HeaderField_value(WS_HDR_PAYLOAD_LENGTH_EXT, raw);
		break;
	case PAYLOAD_EXTENDED_CONTINUED:
		ws->frame.payload_offset += PAYLOAD_EXTENDED_CONTINUED_SIZE;
		assert(StringBuffer_used(ws->in) > ws->frame.payload_offset); /* avoid memory corruption */
		ws->frame.payload_length_extended = HeaderField_value(WS_HDR_PAYLOAD_LENGTH_EXT_CONTINUED, raw);
		break;
	default:
		ws->frame.payload_length_extended = ws->frame.payload_length;
	}
}

static void
WebsocketsFrame_log(WebsocketsFrame* frame)
{
	XFDBG("\n"
	      "--->	fin:%u, rsv1:%u, rsv2:%u, rsv3:%u, masked:%u, opcode:0x%x\n"
	      "	payload_length:%u, payload_length_extended:%llu",
	      frame->fin, frame->rsv1, frame->rsv2, frame->rsv3, frame->masked, frame->opcode,
	      frame->payload_length, frame->payload_length_extended);
}

static int
process_control_frame(Websockets* ws)
{
	/*
	   All control frames MUST have a payload length of 125 bytes or less
	   and MUST NOT be fragmented.
	 */
	if (ws->frame.payload_length < PAYLOAD_EXTENDED)
	{
		switch (ws->frame.opcode)
		{
		case WS_FRAME_CLOSE:
			XFDBG("Received WS_FRAME_CLOSE masked:%u", ws->frame.masked);
			/* TODO send close frame */

			break;
		case WS_FRAME_PING:
			XDBG("Received WS_FRAME_PING");
			break;
		case WS_FRAME_PONG:
			XDBG("Received WS_FRAME_PONG");
			break;
		default:
			break;
		}
		return 1;
	}
	else
	{
		XFDBG("Invalid payload length for control package %hhu", ws->frame.payload_length);
		// TODO send error
		return -1;
	}
}

#define FRAME_HEX_NPRINT 16

static int
Websockets_echo(Websockets* ws)
{
	XFDBG("payload offset:%hhu length:%llu", ws->frame.payload_offset, ws->frame.payload_length_extended);

	/* unmask masked text data */
	if (ws->frame.masked && ws->frame.payload_length > 0)
		WebsocketsFrame_unmask_payload_data(ws);
	else
		XDBG("Message with empty payload");

	switch (ws->frame.opcode)
	{
	case WS_FRAME_CONTINUATION:
		break;
	case WS_FRAME_TEXT:
		WebsocketsFrame_create(ws->out, WS_FRAME_TEXT, (char*)ws->frame.payload_raw, ws->frame.payload_length_extended);
		break;
	case WS_FRAME_BINARY:
		WebsocketsFrame_create(ws->out, WS_FRAME_BINARY, (char*)ws->frame.payload_raw, ws->frame.payload_length_extended);
		break;
	default:
		break; /* error, just to make the compiler happy */
	}

	// echo frame
	StringBuffer_print_bytes_hex(ws->out, FRAME_HEX_NPRINT, "output message");

	return 1;
}

/* extract header fields and calculate offset */
void
WebsocketsFrame_parse_header(Websockets* ws)
{
	assert(StringBuffer_used(ws->in) >= 2);
	uint8_t* raw = (uint8_t*)StringBuffer_value(ws->in);

	ws->frame.payload_length = HeaderField_byte_value(WS_HDR_PAYLOAD_LENGTH, raw);
	ws->frame.payload_offset = 2;
	ws->frame.opcode = HeaderField_byte_value(WS_HDR_OPCODE, raw);
	ws->frame.masked = HeaderField_byte_value(WS_HDR_MASKED, raw);
	ws->frame.fin = HeaderField_byte_value(WS_HDR_FIN, raw);
	ws->frame.rsv1 = HeaderField_byte_value(WS_HDR_RSV1, raw);
	ws->frame.rsv2 = HeaderField_byte_value(WS_HDR_RSV2, raw);
	ws->frame.rsv3 = HeaderField_byte_value(WS_HDR_RSV3, raw);
	/* extract extended payload length */
	WebsocketsFrame_parse_payload_length_extended(ws);
	/* extract masking key */
	if (ws->frame.masked)
		ws->frame.payload_offset += WS_MASKING_KEY_LENGTH;
	ws->frame.length = ws->frame.payload_offset + ws->frame.payload_length_extended;

	WebsocketsFrame_log(&ws->frame);
}

int
WebsocketsFrame_parse(Websockets* ws)
{
	/* frame is complete so we can set the data pointers */
	uint8_t* raw = (uint8_t*)StringBuffer_value(ws->in);

	if (ws->frame.masked)
		ws->frame.masking_key = raw + ws->frame.payload_offset - WS_MASKING_KEY_LENGTH;

	/* calculate final payload offset */
	ws->frame.payload_raw = raw + ws->frame.payload_offset;
	ws->frame.payload_raw_end = ws->frame.payload_raw + ws->frame.payload_length_extended;

	/* ensure we are not corrupting memory */
	assert(ws->frame.payload_raw <= ws->frame.payload_raw_end);
	assert(ws->frame.payload_raw_end <= (uint8_t*)S_term(ws->in->string));

	StringBuffer_print_bytes_hex(ws->in, FRAME_HEX_NPRINT, "package bytes");

	switch (ws->frame.opcode)
	{
	case WS_FRAME_CONTINUATION:
	case WS_FRAME_TEXT:
	case WS_FRAME_BINARY:
		return Websockets_echo(ws);
	case WS_FRAME_CLOSE:
	case WS_FRAME_PING:
	case WS_FRAME_PONG:
		return process_control_frame(ws);
	default:
		// TODO close connection with error
		XFDBG("Invalid opcode: 0x%x", ws->frame.opcode);
		return -1;
	}
}

/*
 * TODO When masked is set to 1 then payload is masked and masking key is included in the payload.
 */
void
WebsocketsFrame_write_to_buffer(StringBuffer* buf, uint8_t header_bits, const char* payload, uint64_t nchars, unsigned int masked)
{
	uint8_t header[2];

	header[0] = header_bits;

	uint8_t payload_offset = 2;
	uint8_t payload_length;

	/* write header with payload length */
	if (nchars <  PAYLOAD_EXTENDED)
		payload_length = (uint8_t)nchars;
	else if (nchars <= PAYLOAD_EXTENDED_MAX)
		payload_length = PAYLOAD_EXTENDED;
	else
		payload_length = PAYLOAD_EXTENDED_CONTINUED;

	XFDBG("Creating response with: %llu chars, payload length:%d", nchars, payload_length);
	/* TODO debug response */

	header[1] = payload_length;
	if (masked)
		header[1] |= (uint8_t)WS_HDR_MASKED.bitmask;

	StringBuffer_ncat(buf, (char*)header, 2);

	if (payload_length > 0)
	{
		/* write extended payload length */
		switch (payload_length)
		{
		case PAYLOAD_EXTENDED:
			payload_offset += StringBuffer_cat_htons(buf, (uint16_t)nchars);
			break;
		case PAYLOAD_EXTENDED_CONTINUED:
			payload_offset += StringBuffer_cat_hton64(buf, nchars);
			break;
		}

		/* write payload */
		StringBuffer_ncat(buf, payload, nchars);
	}
}
