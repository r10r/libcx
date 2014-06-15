#include "frame.h"

static unsigned int
WebsocketsFrame_offset(uint64_t nchars, unsigned int masked);

static uint8_t
WebsocketsFrame_parse_offset(uint8_t payload_length, unsigned int masked);

static inline void
WebsocketsFrame_write_header(StringBuffer* buf, uint8_t header_bits, uint64_t nchars, unsigned int masked);

static inline StringBuffer*
WebsocketsFrame_create_buffer(uint8_t header_bits, uint64_t nchars);

WebsocketsFrame*
WebsocketsFrame_dup(WebsocketsFrame* frame)
{
	WebsocketsFrame* dup = cx_alloc(sizeof(WebsocketsFrame));

	memcpy(dup, frame, sizeof(WebsocketsFrame));
	dup->data = cx_alloc(frame->length);
	memcpy(dup->data, frame->raw, frame->length);
	WebsocketsFrame_parse(frame, dup->data);
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
		XDBG("Unmasking payload data");
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
WebsocketsFrame_parse_payload_length_extended(WebsocketsFrame* frame, char* raw)
{
	XDBG("Parse frame extended payload length");
	switch (frame->payload_length)
	{
	case PAYLOAD_EXTENDED:
		frame->payload_length_extended = HeaderField_value(WS_HDR_PAYLOAD_LENGTH_EXT, raw);
		break;
	case PAYLOAD_EXTENDED_CONTINUED:
		frame->payload_length_extended = HeaderField_value(WS_HDR_PAYLOAD_LENGTH_EXT_CONTINUED, raw);
		break;
	default:
		frame->payload_length_extended = frame->payload_length;
	}
	frame->length = frame->payload_offset + frame->payload_length_extended;
	XFDBG("Parsed extended payload size: %llu", frame->payload_length_extended);
}

uint16_t
WebsocketsFrame_response_status(WebsocketsFrame* frame)
{
	XDBG("Parse frame status");
	uint16_t response_status;

	if (frame->payload_length == 0)
	{
		XDBG("Close frame has no status code");
		response_status = htons(WS_CODE_SUCCESS);
	}
	else if (frame->payload_length == 1)
	{
		XDBG("Close frame has incomplete status code (payload length 1, required 2)");
		response_status = htons(WS_CODE_ERROR_PROTOCOL);
	}
	else
	{
		uint16_t close_code = ntohs(*((uint16_t*)(frame->payload_raw)));

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
	return response_status;
}

StringBuffer*
WebsocketsFrame_create_echo(WebsocketsFrame* frame)
{
	XDBG("Create echo frame");
	switch (frame->opcode)
	{
	case WS_FRAME_CONTINUATION:
		/* TODO check if it is possible send echo on a continuation frame */
		break;
	case WS_FRAME_BINARY:
	case WS_FRAME_TEXT:
		return WebsocketsFrame_create(frame->opcode, (char*)frame->payload_raw, frame->payload_length_extended);
	default:
		break;
	}
	return NULL;
}

/* extract header fields and calculate offset */
void
WebsocketsFrame_parse_header(WebsocketsFrame* frame, char* raw)
{
	XDBG("Parse frame header");
	frame->payload_length = HeaderField_byte_value(WS_HDR_PAYLOAD_LENGTH, raw);
	frame->opcode = HeaderField_byte_value(WS_HDR_OPCODE, raw);
	frame->masked = HeaderField_byte_value(WS_HDR_MASKED, raw);
	frame->fin = HeaderField_byte_value(WS_HDR_FIN, raw);
	frame->rsv1 = HeaderField_byte_value(WS_HDR_RSV1, raw);
	frame->rsv2 = HeaderField_byte_value(WS_HDR_RSV2, raw);
	frame->rsv3 = HeaderField_byte_value(WS_HDR_RSV3, raw);
	frame->payload_offset = WebsocketsFrame_parse_offset(frame->payload_length, frame->masked);

	if (frame->payload_length <= PAYLOAD_MAX)
		frame->length = frame->payload_offset + frame->payload_length;

	WebsocketsFrame_log(frame);
}

void
WebsocketsFrame_parse(WebsocketsFrame* frame, uint8_t* raw)
{
	XDBG("Parse frame");
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

StringBuffer*
WebsocketsFrame_create(uint8_t opcode, const char* payload, uint64_t nchars)
{
	XDBG("Creating frame");
	uint8_t header_bits = (uint8_t)WS_HDR_FIN.bitmask | opcode;
	StringBuffer* buf = WebsocketsFrame_create_buffer(header_bits, nchars);

	if (nchars)
	{
		assert(nchars <= UINT32_MAX);         /* FIXME make StringBuffer_append accept uint64_t as length */
		StringBuffer_ncat(buf, payload, (uint32_t)nchars);
	}
	else
	{
		XDBG("No payload to send.");
	}

	return buf;
}

StringBuffer*
WebsocketsFrame_create_error(WebsocketsStatusCode status_code, const char* message)
{
	XDBG("Creating error frame");
	size_t nchars = WS_STATUS_CODE_SIZE + strlen(message);

	assert(nchars <= WS_CONTROL_MESSAGE_SIZE_MAX); /* application error */

	uint8_t header_bits = (uint8_t)WS_HDR_FIN.bitmask | WS_FRAME_CLOSE;

	StringBuffer* buf = WebsocketsFrame_create_buffer(header_bits, nchars);
	StringBuffer_cat_htons(buf, status_code);
	StringBuffer_ncat(buf, message, nchars);

	assert(StringBuffer_length(buf) <= WS_CONTROL_MESSAGE_SIZE_MAX); /* application error */

	return buf;
}

static uint8_t
WebsocketsFrame_parse_offset(uint8_t payload_length, unsigned int masked)
{
	uint8_t offset = WS_HEADER_SIZE;

	if (payload_length == PAYLOAD_EXTENDED)
	{
		offset += 2;
	}
	else if (payload_length == PAYLOAD_EXTENDED_CONTINUED)
	{
		offset += 8;
	}

	if (masked)
		offset += WS_MASKING_KEY_LENGTH;

	XFDBG("Parsed frame offset: %u (payload field:%u, masked:%u)", offset, payload_length, masked);
	return offset;
}

static unsigned int
WebsocketsFrame_offset(uint64_t nchars, unsigned int masked)
{
	unsigned int offset = WS_HEADER_SIZE;

	if (nchars >  PAYLOAD_EXTENDED_MAX)
	{
		offset += 8;
	}
	else if (nchars > PAYLOAD_MAX)
	{
		offset += 2;
	}

	if (masked)
		offset += WS_MASKING_KEY_LENGTH;

	XFDBG("Calculated frame offset: %u (length:%llu, masked:%u)", offset, nchars, masked);
	return offset;
}

static inline StringBuffer*
WebsocketsFrame_create_buffer(uint8_t header_bits, uint64_t nchars)
{
	unsigned int offset = WebsocketsFrame_offset(nchars, 0);
	StringBuffer* buf = StringBuffer_new(offset + nchars);

	WebsocketsFrame_write_header(buf, header_bits, nchars, 0);
	return buf;
}

/*
 * TODO When masked is set to 1 then payload is masked and masking key is included in the payload.
 */
static inline void
WebsocketsFrame_write_header(StringBuffer* buf, uint8_t header_bits, uint64_t nchars, unsigned int masked)
{
	uint8_t header[2];

	header[0] = header_bits;

	uint8_t payload_length;

	/* write header with payload length */
	if (nchars <  PAYLOAD_EXTENDED)
	{
		payload_length = (uint8_t)nchars;
	}
	else if (nchars <= PAYLOAD_EXTENDED_MAX)
	{
		payload_length = PAYLOAD_EXTENDED;
	}
	else
	{
		payload_length = PAYLOAD_EXTENDED_CONTINUED;
	}

	XFDBG("Creating response with: %llu chars, payload length:%d", nchars, payload_length);

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
			StringBuffer_cat_htons(buf, (uint16_t)nchars);
			break;
		case PAYLOAD_EXTENDED_CONTINUED:
			StringBuffer_cat_hton64(buf, nchars);
			break;
		}
	}
}
