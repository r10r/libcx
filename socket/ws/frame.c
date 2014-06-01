#include "frame.h"

//void
//wsMakeFrame(Websockets* ws, enum wsFrameType frameType, uint8_t* payload, uint64_t payload_length)
//{
//	/* ensure frameType is not an error frame type  ? */
////	assert(frameType < WS_EMPTY_FRAME /* first error frame */);
//
//	/* clear buffer */
//	StringBuffer_clear(ws->out);
//	size_t frame_header_length = 2;
//	uint8_t* frame_buf = (uint8_t*)StringBuffer_value(ws->out);
//
//	frame_buf[0] =  (0x80 | frameType);
//
//	if (payload_length < EXTENDED_PAYLOAD_LENGTH)
//		frame_buf[1] = (uint8_t)payload_length;
//	else if (payload_length <= EXTENDED_PAYLOAD_MAX)
//	{
//		frame_buf[1] = EXTENDED_PAYLOAD_LENGTH;
//		uint16_t payload_length_net = htons(payload_length);
//		memcpy(frame_buf[2], &payload_length_net, sizeof(payload_length_net));
//		frame_header_length += sizeof(payload_length_net);
//	}
//	else
//	{
//		frame_buf[1] = CONTINUED_EXTENDED_PAYLOAD_LENGTH;
//		uint64_t payload_length_net = htobe64(payload_length);
//		memcpy(frame_buf[2], &payload_length_net, sizeof(payload_length_net));
//		frame_header_length += sizeof(payload_length_net);
//	}
//
//	/* copy data */
//	// ensure output buffer is large enough
//	size_t package_length = frame_header_length + payload_length;
//	StringBuffer_make_room(ws->out, 0, package_length + 1);
//
//	/* copy input data */
//	memcpy(frame_buf[frame_header_length], payload, payload_length);
//
//	// terminate buffer
//	frame_buf[package_length] = '\0';
//	ws->out->string->length = package_length + 1;
//}

void
Websockets_parse_input_frame(Websockets* ws)
{
//	uint8_t inputFrame = StringBuffer_value(ws->in)[0];
//
//	ws->

//	if ((inputFrame[0] & 0x70) != 0x0)      // checks extensions off
//		return WS_ERROR_FRAME;
//	if ((inputFrame[0] & 0x80) != 0x80)     // we haven't continuation frames support
//		return WS_ERROR_FRAME;                                  // so, fin flag must be set
//		int masked = wsinputFrame[1] & 0x80) != 0x80)     // checks masking bit
//		return WS_ERROR_FRAME;

//	uint8_t opcode = StringBuffer_value(ws->in)[0] & 0x0F;
//
//	if (opcode == WS_TEXT_FRAME ||
//	    opcode == WS_BINARY_FRAME ||
//	    opcode == WS_CLOSING_FRAME ||
//	    opcode == WS_PING_FRAME ||
//	    opcode == WS_PONG_FRAME
//	    )
//	{
//		enum wsFrameType frameType = opcode;
//
//		uint64_t payload_length = getPayloadLength(ws);
//
//		XFDBG("Frame: opcode:%#x payload:%zu, input:%zu, extra:%u", opcode, payload_length);
//		if (payload_length > 0)
//		{
////			if (payloadLength < inputLength - 6 - payloadFieldExtraBytes) // 4-maskingKey, 2-header
////				return WS_INCOMPLETE_FRAME;
//
//			uint8_t* maskingKey = &inputFrame[2 + payloadFieldExtraBytes];
//
////			assert(payloadLength == inputLength - 6 - payloadFieldExtraBytes);
//
//			*dataPtr = &inputFrame[2 + payloadFieldExtraBytes + 4];
//			*dataLength = payload_length;
//
//			size_t i;
//			for (i = 0; i < *dataLength; i++)
//				(*dataPtr)[i] = (*dataPtr)[i] ^ maskingKey[i % 4];
//		}
//		return frameType;
//	}
//
//	return WS_ERROR_FRAME;
}
