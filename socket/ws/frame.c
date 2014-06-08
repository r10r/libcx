#include "frame.h"

WebsocketsFrame*
WebsocketsFrame_dup(WebsocketsFrame* frame)
{
	WebsocketsFrame* dup = cx_alloc(sizeof(WebsocketsFrame));

	memcpy(dup, frame, sizeof(WebsocketsFrame));
	dup->data = cx_alloc(frame->length);
	memcpy(dup->data, frame->raw, frame->length);
	WebsocketsFrame_parse(frame, dup->data, frame->length);
	return dup;
}

void
WebsocketsFrame_free(WebsocketsFrame* frame)
{
	if (frame->data)
		cx_free(frame->data);
	cx_free(frame);
}

void
WebsocketsFrame_unmask_payload_data(WebsocketsFrame* frame)
{
	if (frame->masked && frame->payload_length > 0)
	{
		XDBG("Unmasking data");
		size_t i = 0;
		uint8_t* payload_byte = frame->payload_raw;

		for (i = 0; i < frame->payload_length_extended; i++)
		{
			/* we are corrupting memory */
			assert(payload_byte < frame->payload_raw_end);

			uint8_t key = *(frame->masking_key + (i % WS_MASKING_KEY_LENGTH));
			*payload_byte = *payload_byte ^ key;
			payload_byte++;
		}
	}
	else
		XDBG("Message with empty payload");
}

void
WebsocketsFrame_parse_payload_length_extended(WebsocketsFrame* frame, char* raw, size_t length)
{
	switch (frame->payload_length)
	{
	case PAYLOAD_EXTENDED:
		frame->payload_offset += PAYLOAD_EXTENDED_SIZE;
		assert(length > frame->payload_offset); /* avoid memory corruption */
		frame->payload_length_extended = HeaderField_value(WS_HDR_PAYLOAD_LENGTH_EXT, raw);
		break;
	case PAYLOAD_EXTENDED_CONTINUED:
		frame->payload_offset += PAYLOAD_EXTENDED_CONTINUED_SIZE;
		assert(length > frame->payload_offset); /* avoid memory corruption */
		frame->payload_length_extended = HeaderField_value(WS_HDR_PAYLOAD_LENGTH_EXT_CONTINUED, raw);
		break;
	default:
		frame->payload_length_extended = frame->payload_length;
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

static void
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
		{
			XFDBG("Received WS_FRAME_CLOSE masked:%u", ws->frame.masked);

			uint16_t response_status;

			if (ws->frame.payload_length == 0)
			{
				XDBG("Close frame has no status code");
				response_status = htons(WS_CODE_SUCCESS);
			}
			else if (ws->frame.payload_length == 1)
			{
				XDBG("Close frame has incomplete status code (payload length 1, required 2)");
				response_status = htons(WS_CODE_ERROR_PROTOCOL);
			}
			else if (ws->frame.payload_length > 1)
			{
				uint16_t close_code = ntohs(*((uint16_t*)(ws->frame.payload_raw)));

				if (close_code >= WS_CODE_PROTOCOL_MIN && close_code <= WS_CODE_PROTOCOL_MAX)
				{
					switch (close_code)
					{
					case WS_CODE_MOVED:
					case WS_CODE_CLIENT_MISSING_EXTENSION:
					case WS_CODE_ERROR_MESSAGE_TYPE:
					case WS_CODE_ERROR_MESSAGE_VALUE:
					case WS_CODE_ERROR_PROTOCOL:
					case WS_CODE_POLICY_MESSAGE_TO_BIG:
					case WS_CODE_POLICY_VIOLATION:
					case WS_CODE_SERVER_ERROR:
					case WS_CODE_SUCCESS:
						XFDBG("Close frame status code: %d", close_code);
						response_status = htons(WS_CODE_SUCCESS);
						break;
					default:
						XFDBG("Invalid close frame status code: %d", close_code);
						response_status = htons(WS_CODE_ERROR_PROTOCOL);
					}
				}
				else if (close_code >= WS_CODE_PUBLIC_MIN && close_code <= WS_CODE_PUBLIC_MAX)
					/* reply with received public status code */
					response_status = htons(close_code);
				else if (close_code >= WS_CODE_PRIVATE_MIN && close_code <= WS_CODE_PRIVATE_MAX)
					/* reply with received private status code */
					response_status = htons(close_code);
				else
				{
					XFDBG("Invalid close frame status code: %d", close_code);
					response_status = htons(WS_CODE_ERROR_PROTOCOL);
				}
			}

			WebsocketsFrame_create(ws->out, WS_FRAME_CLOSE, (char*)&response_status, 2);
			ws->state = WS_STATE_CLOSE;
			break;
		}
		case WS_FRAME_PING:
		{
			XDBG("Received WS_FRAME_PING");
			/* send PONG frame with unmasked payload data from PING frame */
			XFDBG("ping frame: payload offset:%hhu length:%llu", ws->frame.payload_offset, ws->frame.payload_length_extended);
			WebsocketsFrame_create(ws->out, WS_FRAME_PONG, (char*)ws->frame.payload_raw, ws->frame.payload_length_extended);
			ws->state = WS_STATE_FRAME_SEND_RESPONSE; /* FIXME move this marker to the buffer (e.g a flush flag) */
			break;
		}
		case WS_FRAME_PONG:
		{
			XERR("Ignoring unsolicitated PONG frame.");
			StringBuffer_shift(ws->in, (uint32_t)ws->frame.length);
			ws->state = WS_STATE_FRAME_NEW;
			break;
		}
		default:
			break;
		}
	}

	else
	{
		Websockets_ferror(ws, WS_CODE_ERROR_PROTOCOL,
				  "Invalid payload length for control package %hhu", ws->frame.payload_length)
	}
}

static void
Websockets_echo(Websockets* ws)
{
	XFDBG("payload offset:%hhu length:%llu", ws->frame.payload_offset, ws->frame.payload_length_extended);

	switch (ws->frame.opcode)
	{
	case WS_FRAME_CONTINUATION:
		break;
	case WS_FRAME_TEXT:
		WebsocketsFrame_create(ws->out, WS_FRAME_TEXT, (char*)ws->frame.payload_raw, ws->frame.payload_length_extended);
		ws->state = WS_STATE_FRAME_SEND_RESPONSE;
		break;
	case WS_FRAME_BINARY:
		WebsocketsFrame_create(ws->out, WS_FRAME_BINARY, (char*)ws->frame.payload_raw, ws->frame.payload_length_extended);
		ws->state = WS_STATE_FRAME_SEND_RESPONSE;
		break;
	default:
		break; /* error, just to make the compiler happy */
	}
}

/* extract header fields and calculate offset */
void
WebsocketsFrame_parse_header(WebsocketsFrame* frame, char* raw, size_t length)
{
	frame->payload_length = HeaderField_byte_value(WS_HDR_PAYLOAD_LENGTH, raw);
	frame->payload_offset = 2;
	frame->opcode = HeaderField_byte_value(WS_HDR_OPCODE, raw);
	frame->masked = HeaderField_byte_value(WS_HDR_MASKED, raw);
	frame->fin = HeaderField_byte_value(WS_HDR_FIN, raw);
	frame->rsv1 = HeaderField_byte_value(WS_HDR_RSV1, raw);
	frame->rsv2 = HeaderField_byte_value(WS_HDR_RSV2, raw);
	frame->rsv3 = HeaderField_byte_value(WS_HDR_RSV3, raw);
	/* extract extended payload length */
	WebsocketsFrame_parse_payload_length_extended(frame, raw, length);
	/* extract masking key */
	if (frame->masked)
		frame->payload_offset += WS_MASKING_KEY_LENGTH;
	frame->length = frame->payload_offset + frame->payload_length_extended;

	WebsocketsFrame_log(frame);
}

void
WebsocketsFrame_parse(WebsocketsFrame* frame, uint8_t* raw, size_t length)
{
	/* frame is complete so we can set the data pointers */
	frame->raw = raw;

	if (frame->masked)
		frame->masking_key = raw + frame->payload_offset - WS_MASKING_KEY_LENGTH;

	/* calculate final payload offset */
	frame->payload_raw = raw + frame->payload_offset;
	frame->payload_raw_end = frame->payload_raw + frame->payload_length_extended;

	/* ensure we are not corrupting memory */
	assert(frame->payload_raw <= frame->payload_raw_end);
}

void
WebsocketsFrame_process(Websockets* ws)
{
	switch (ws->frame.opcode)
	{
	case WS_FRAME_CONTINUATION:
//		if (ws->last_frament)
//		{
//			// copy last fragment
//		}
//			WebsocketsFrame_unmask_payload_data(&ws->frame);
		break;

	case WS_FRAME_TEXT:
	case WS_FRAME_BINARY:
		WebsocketsFrame_unmask_payload_data(&ws->frame);
		Websockets_echo(ws);
		break;
	case WS_FRAME_CLOSE:
	case WS_FRAME_PING:
	case WS_FRAME_PONG:
		WebsocketsFrame_unmask_payload_data(&ws->frame);
		process_control_frame(ws);
		break;
	default:
		Websockets_ferror(ws, WS_CODE_ERROR_PROTOCOL, "Invalid opcode: 0x%x", ws->frame.opcode)
	}
}

void
WebsocketsFrame_write_error(Websockets* ws)
{
	/* error message length + 2 (status code length) must be < 126 */
	assert(StringBuffer_used(ws->error_message) < PAYLOAD_EXTENDED - 2);

	StringBuffer* buf = StringBuffer_new(125);
	StringBuffer_cat_htons(buf, ws->status_code);
	StringBuffer_ncat(buf, StringBuffer_value(ws->error_message), StringBuffer_used(ws->error_message));

	WebsocketsFrame_write_to_buffer(ws->out, (uint8_t)WS_HDR_FIN.bitmask | WS_FRAME_CLOSE,
					StringBuffer_value(buf), StringBuffer_used(buf), 0);

	StringBuffer_free(buf);
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
		assert(nchars <= UINT32_MAX); /* FIXME make StringBuffer_append accept uint64_t as length */
		StringBuffer_ncat(buf, payload, (uint32_t)nchars);
	}
	else
		XDBG("No payload to send.");
}
