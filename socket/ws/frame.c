#include "frame.h"

void
WebsocketsFrame_unmask_payload_data(Websockets* ws, uint8_t* masking_key)
{
	size_t i = 0;
	uint8_t* payload_byte = ws->frame.payload_raw;

	for (i = 0; i < ws->frame.payload_length; i++)
	{
		/* we are corrupting memory */
		assert(payload_byte <= ws->frame.payload_raw_last);

		uint8_t key = *(masking_key + (i % WS_MASKING_KEY_LENGTH));
//		printf("masking_key: %u 0x%x\n", key, key);

		*payload_byte = *payload_byte ^ key;
		payload_byte++;
	}
}

void
WebsocketsFrame_parse_extended_payload_length(Websockets* ws)
{
	switch (ws->frame.payload_length)
	{
	case PAYLOAD_EXTENDED:
		ws->frame.payload_length = HeaderField_value(WS_HDR_PAYLOAD_LENGTH_EXT, ws->frame.raw);
		ws->frame.payload_offset += PAYLOAD_EXTENDED_SIZE;
		break;
	case PAYLOAD_EXTENDED_CONTINUED:
		ws->frame.payload_length = HeaderField_value(WS_HDR_PAYLOAD_LENGTH_EXT_CONTINUED, ws->frame.raw);
		ws->frame.payload_offset += PAYLOAD_EXTENDED_CONTINUED_SIZE;
		break;
	}
}

static void
WebsocketsFrame_log(Websockets* ws)
{
	printf("opcode: %u\n", HeaderField_byte_value(WS_HDR_OPCODE, ws->frame.raw));
	printf("masked: %u\n", HeaderField_byte_value(WS_HDR_MASKED, ws->frame.raw));
	printf("fin: %u\n", HeaderField_byte_value(WS_HDR_FIN, ws->frame.raw));
	printf("rsv1: %u\n", HeaderField_byte_value(WS_HDR_RSV1, ws->frame.raw));
	printf("rsv2: %u\n", HeaderField_byte_value(WS_HDR_RSV2, ws->frame.raw));
	printf("rsv3: %u\n", HeaderField_byte_value(WS_HDR_RSV3, ws->frame.raw));
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
		case WS_FRAME_PING:
		case WS_FRAME_PONG:
		default:
			break;
		}
		return 1;
	}
	else
		return -1;
}

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

	WebsocketsFrame_parse_extended_payload_length(ws);

	uint8_t masked = HeaderField_byte_value(WS_HDR_MASKED, ws->frame.raw);

	if (masked)
		ws->frame.payload_offset += WS_MASKING_KEY_LENGTH;

	ws->frame.payload_raw = ws->frame.raw + ws->frame.payload_offset;
	ws->frame.payload_raw_last = ws->frame.payload_raw + ws->frame.payload_length;

	/* ensure we are not corrupting memory */
	assert(ws->frame.payload_raw_last <= (uint8_t*)S_last(ws->in->string));

	/* unmask input data */
	if (masked)
	{
		uint8_t* masking_key = ws->frame.payload_raw - WS_MASKING_KEY_LENGTH;
		WebsocketsFrame_unmask_payload_data(ws, masking_key);
	}

	printf("payload offset:%hhu length:%llu [%s]\n",
	       ws->frame.payload_offset, ws->frame.payload_length,
	       StringBuffer_at(ws->in, ws->frame.payload_offset));

	return 1;
}

int
WebsocketsFrame_parse(Websockets* ws)
{
	ws->frame.raw = (uint8_t*)StringBuffer_value(ws->in);
	ws->frame.payload_length = HeaderField_byte_value(WS_HDR_PAYLOAD_LENGTH, ws->frame.raw);
	ws->frame.payload_offset = 2;

	StringBuffer_print_bytes_hex(ws->in, StringBuffer_used(ws->in), "package bytes");
	WebsocketsFrame_log(ws);

	/* decode opcode */
	uint8_t opcode = HeaderField_byte_value(WS_HDR_OPCODE, ws->frame.raw);

	switch (opcode)
	{
	case WS_FRAME_CONTINUATION:
	case WS_FRAME_TEXT:
	case WS_FRAME_BINARY:
		ws->frame.opcode = (WebsocketsOpcode)opcode;
		return process_data_frame(ws);
	case WS_FRAME_CLOSE:
	case WS_FRAME_PING:
	case WS_FRAME_PONG:
		ws->frame.opcode = (WebsocketsOpcode)opcode;
		return process_control_frame(ws);
	default:
		// TODO close connection with error
		printf("Invalid opcode: 0x%x\n", opcode);
		return -1;
	}
}

static void
WebsocketsFrame_create(Websockets* ws)
{
//	WebsocketsFrame response;

//	response.raw = (uint8_t*)StringBuffer_value(ws->out);
//	response.payload_offset = 2;
//
//	*(response.raw) = BIT(7) | WS_FRAME_TEXT;
//
//	if (ws->frame.payload_length <  EXTENDED_PAYLOAD_LENGTH)
//		*(response.raw + 1) = EXTENDED_PAYLOAD_LENGTH;
//	else if (ws->frame.payload_length < EXTENDED_PAYLOAD_MAX)
//	{
//		*(response.raw + 1) = EXTENDED_PAYLOAD_LENGTH;
//		response.payload_offset += 2;
//		*((uint16_t*)(response.raw + 2)) = htons(ws->frame.payload_length);
//	}
//	else
//	{
//		*(response.raw + 1) = CONTINUED_EXTENDED_PAYLOAD_LENGTH;
//		response.payload_offset += 8;
//		*((uint64_t*)(response.raw + 2)) = hton64(ws->frame.payload_length);
//	}
//
//	ws->out->string->length = response.payload_length;
//	S_nullterm(ws->out->string);
//	StringBuffer_append(ws->out, response.payload_offset, (char*)ws->frame.payload_raw, ws->frame.payload_length);

	*(ws->frame.raw) = BIT_CLEAR(*(ws->frame.payload_raw + 1), 7);
	StringBuffer_append(ws->in, 2, (char*)ws->frame.payload_raw, ws->frame.payload_length);

	StringBuffer_print_bytes_hex(ws->in, StringBuffer_used(ws->in), "output message");

	StringBuffer_clear(ws->out);
}
