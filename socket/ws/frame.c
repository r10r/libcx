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
	switch (ws->frame.payload_length)
	{
	case PAYLOAD_EXTENDED:
		ws->frame.payload_length_extended = HeaderField_value(WS_HDR_PAYLOAD_LENGTH_EXT, ws->frame.raw);
		ws->frame.payload_offset += PAYLOAD_EXTENDED_SIZE;
		break;
	case PAYLOAD_EXTENDED_CONTINUED:
		ws->frame.payload_length_extended = HeaderField_value(WS_HDR_PAYLOAD_LENGTH_EXT_CONTINUED, ws->frame.raw);
		ws->frame.payload_offset += PAYLOAD_EXTENDED_CONTINUED_SIZE;
		break;
	default:
		ws->frame.payload_length_extended = ws->frame.payload_length;
	}
}

static void
WebsocketsFrame_log(Websockets* ws)
{
	XFDBG("opcode: %u", ws->frame.opcode);
	XFDBG("masked: %u", ws->frame.masked);
	XFDBG("fin: %u", HeaderField_byte_value(WS_HDR_FIN, ws->frame.raw));
	XFDBG("rsv1: %u", HeaderField_byte_value(WS_HDR_RSV1, ws->frame.raw));
	XFDBG("rsv2: %u", HeaderField_byte_value(WS_HDR_RSV2, ws->frame.raw));
	XFDBG("rsv3: %u", HeaderField_byte_value(WS_HDR_RSV3, ws->frame.raw));
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
process_data_frame(Websockets* ws)
{
	switch (ws->frame.opcode)
	{
	case WS_FRAME_CONTINUATION:
	case WS_FRAME_TEXT:
	case WS_FRAME_BINARY:
	default:
		break;
	}

	XFDBG("payload offset:%hhu length:%llu", ws->frame.payload_offset, ws->frame.payload_length_extended);

	/* unmask masked text data */
	if (ws->frame.masked && ws->frame.payload_length > 0)
		WebsocketsFrame_unmask_payload_data(ws);
	else
		XDBG("Message with empty payload");

	// echo frame
	WebsocketsFrame_create_data_frame(ws->out, (char*)ws->frame.payload_raw, ws->frame.payload_length_extended, ws->frame.opcode);
	StringBuffer_print_bytes_hex(ws->out, FRAME_HEX_NPRINT, "output message");

	return 1;
}

int
WebsocketsFrame_parse(Websockets* ws)
{
	assert(StringBuffer_used(ws->in) >= 2);

	/* string buffer is resized so every time there is new data we have to reset all data pointers */
	ws->frame.raw = (uint8_t*)StringBuffer_value(ws->in);

	if (ws->state == WS_STATE_ESTABLISHED)
	{
		ws->frame.payload_length = HeaderField_byte_value(WS_HDR_PAYLOAD_LENGTH, ws->frame.raw);
		ws->frame.payload_offset = 2;

		ws->frame.opcode = HeaderField_byte_value(WS_HDR_OPCODE, ws->frame.raw);
		ws->frame.masked = HeaderField_byte_value(WS_HDR_MASKED, ws->frame.raw);
		ws->frame.fin = HeaderField_byte_value(WS_HDR_FIN, ws->frame.raw);
		ws->frame.rsv1 = HeaderField_byte_value(WS_HDR_RSV1, ws->frame.raw);
		ws->frame.rsv2 = HeaderField_byte_value(WS_HDR_RSV2, ws->frame.raw);
		ws->frame.rsv3 = HeaderField_byte_value(WS_HDR_RSV3, ws->frame.raw);


		/* extract extended payload length */
		WebsocketsFrame_parse_payload_length_extended(ws);

		/* extract masking key */
		if (ws->frame.masked)
			ws->frame.payload_offset += WS_MASKING_KEY_LENGTH;
	}

	if (ws->frame.masked)
		ws->frame.masking_key = ws->frame.raw + ws->frame.payload_offset - WS_MASKING_KEY_LENGTH;

	/* calculate final payload offset */
	ws->frame.payload_raw = ws->frame.raw + ws->frame.payload_offset;
	ws->frame.payload_raw_end = ws->frame.payload_raw + ws->frame.payload_length_extended;

	/* ensure we are not corrupting memory */
	assert(ws->frame.payload_raw <= ws->frame.payload_raw_end);

	/* check if frame is fully loaded into buffer, if not stop processing here */
	long missing_payload_bytes = (char*)ws->frame.payload_raw_end - S_term(ws->in->string);
	if (missing_payload_bytes > 0)
	{
		XFDBG("incomplete frame: %ld bytes missing", missing_payload_bytes);
		ws->state = WS_STATE_INCOMPLETE;
		return 0;
	}

	/* reset state to established (maybe WS_STATE_COMPLETE is better ?) */
	ws->state = WS_STATE_ESTABLISHED;

	assert(ws->frame.payload_raw_end <= (uint8_t*)S_term(ws->in->string));

	StringBuffer_print_bytes_hex(ws->in, FRAME_HEX_NPRINT, "package bytes");
	WebsocketsFrame_log(ws);

	switch (ws->frame.opcode)
	{
	case WS_FRAME_CONTINUATION:
	case WS_FRAME_TEXT:
	case WS_FRAME_BINARY:
		return process_data_frame(ws);
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

void
WebsocketsFrame_create_data_frame(StringBuffer* buf, const char* payload, uint64_t nchars, WebsocketsOpcode opcode)
{
	uint8_t header[2];
	uint8_t payload_offset = 2;
	uint8_t payload_length;

	header[0] = 0x0 | BIT(7) /* FIN */ | opcode;

	/* write header with payload length */
	if (nchars <  PAYLOAD_EXTENDED)
		payload_length = (uint8_t)nchars;
	else if (nchars <= PAYLOAD_EXTENDED_MAX)
		payload_length = PAYLOAD_EXTENDED;
	else
		payload_length = PAYLOAD_EXTENDED_CONTINUED;

	XFDBG("Creating text response with: %llu chars, payload length:%d", nchars, payload_length);

	header[1] = payload_length;
	StringBuffer_ncat(buf, (char*)header, 2);

	if (payload_length > 0)
	{
		/* write extended payload length */
		if (payload_length ==  PAYLOAD_EXTENDED)
		{
			payload_offset += 2;
			uint16_t payload_length_extended_net = htons((uint16_t)nchars);
			StringBuffer_ncat(buf, (char*)(&payload_length_extended_net), 2);
		}
		else if (payload_length == PAYLOAD_EXTENDED_CONTINUED)
		{
			payload_offset += 8;
			uint64_t payload_length_extended_net = hton64((uint64_t)nchars);
			StringBuffer_ncat(buf, (char*)(&payload_length_extended_net), 8);
		}

		/* write payload */
		StringBuffer_ncat(buf, payload, nchars);
	}
}
