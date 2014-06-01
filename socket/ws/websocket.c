/*
 * Copyright (c) 2014 Putilov Andrey
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include "websocket.h"
#include "base/debug.h"
#include "string/string_buffer.h"
#include <inttypes.h> /* print types uint64_t */
#include "util.h"

static char rn[] PROGMEM = "\r\n";

void
nullHandshake(struct handshake* hs)
{
	hs->host = NULL;
	hs->origin = NULL;
	hs->resource = NULL;
	hs->key = NULL;
	hs->frameType = WS_EMPTY_FRAME;
}

void
freeHandshake(struct handshake* hs)
{
	if (hs->host)
		free(hs->host);
	if (hs->origin)
		free(hs->origin);
	if (hs->resource)
		free(hs->resource);
	if (hs->key)
		free(hs->key);
	nullHandshake(hs);
}

static char*
getUptoLinefeed(const char* startFrom)
{
	char* writeTo = NULL;
	uint8_t newLength = (uint8_t)(strstr_P(startFrom, rn) - startFrom);

	assert(newLength);
	writeTo = (char*)malloc(newLength + 1); //+1 for '\x00'
	assert(writeTo);
	memcpy(writeTo, startFrom, newLength);
	writeTo[ newLength ] = 0;

	return writeTo;
}

enum wsFrameType
wsParseHandshake(const uint8_t* inputFrame, size_t inputLength,
		 struct handshake* hs)
{
	const char* inputPtr = (const char*)inputFrame;
	const char* endPtr = (const char*)inputFrame + inputLength;

	if (!strstr((const char*)inputFrame, "\r\n\r\n"))
		return WS_INCOMPLETE_FRAME;

	if (memcmp_P(inputFrame, PSTR("GET "), 4) != 0)
		return WS_ERROR_FRAME;
	// measure resource size
	char* first = strchr((const char*)inputFrame, ' ');
	if (!first)
		return WS_ERROR_FRAME;
	first++;
	char* second = strchr(first, ' ');
	if (!second)
		return WS_ERROR_FRAME;

	if (hs->resource)
	{
		free(hs->resource);
		hs->resource = NULL;
	}
	hs->resource = (char*)malloc((size_t)(second - first + 1)); // +1 is for \x00 symbol
	assert(hs->resource);

	if (sscanf_P(inputPtr, PSTR("GET %s HTTP/1.1\r\n"), hs->resource) != 1)
		return WS_ERROR_FRAME;
	inputPtr = strstr_P(inputPtr, rn) + 2;

	/*
	    parse next lines
	 */
    #define prepare(x) do { if (x) { free(x); x = NULL; } \
} \
	while (0)
    #define strtolower(x) do { int i; for (i = 0; x[i]; i++) x[i] = (char)tolower(x[i]); } \
	while (0)
	uint8_t connectionFlag = FALSE;
	uint8_t upgradeFlag = FALSE;
	uint8_t subprotocolFlag = FALSE;
	uint8_t versionMismatch = FALSE;
	while (inputPtr < endPtr && inputPtr[0] != '\r' && inputPtr[1] != '\n')
	{
		if (memcmp_P(inputPtr, hostField, strlen_P(hostField)) == 0)
		{
			inputPtr += strlen_P(hostField);
			prepare(hs->host);
			hs->host = getUptoLinefeed(inputPtr);
		}
		else
		if (memcmp_P(inputPtr, originField, strlen_P(originField)) == 0)
		{
			inputPtr += strlen_P(originField);
			prepare(hs->origin);
			hs->origin = getUptoLinefeed(inputPtr);
		}
		else
		if (memcmp_P(inputPtr, protocolField, strlen_P(protocolField)) == 0)
		{
			inputPtr += strlen_P(protocolField);
			subprotocolFlag = TRUE;
		}
		else
		if (memcmp_P(inputPtr, keyField, strlen_P(keyField)) == 0)
		{
			inputPtr += strlen_P(keyField);
			prepare(hs->key);
			hs->key = getUptoLinefeed(inputPtr);
		}
		else
		if (memcmp_P(inputPtr, versionField, strlen_P(versionField)) == 0)
		{
			inputPtr += strlen_P(versionField);
			char* versionString = NULL;
			versionString = getUptoLinefeed(inputPtr);
			if (memcmp_P(versionString, version, strlen_P(version)) != 0)
				versionMismatch = TRUE;
			free(versionString);
		}
		else
		if (memcmp_P(inputPtr, connectionField, strlen_P(connectionField)) == 0)
		{
			inputPtr += strlen_P(connectionField);
			char* connectionValue = NULL;
			connectionValue = getUptoLinefeed(inputPtr);
			strtolower(connectionValue);
			assert(connectionValue);
			if (strstr_P(connectionValue, upgrade) != NULL)
				connectionFlag = TRUE;
			free(connectionValue);
		}
		else
		if (memcmp_P(inputPtr, upgradeField, strlen_P(upgradeField)) == 0)
		{
			inputPtr += strlen_P(upgradeField);
			char* compare = NULL;
			compare = getUptoLinefeed(inputPtr);
			strtolower(compare);
			assert(compare);
			if (memcmp_P(compare, websocket, strlen_P(websocket)) == 0)
				upgradeFlag = TRUE;
			free(compare);
		}
		;

		inputPtr = strstr_P(inputPtr, rn) + 2;
	}

	// we have read all data, so check them
	if (!hs->host || !hs->key || !connectionFlag || !upgradeFlag || subprotocolFlag
	    || versionMismatch)
		hs->frameType = WS_ERROR_FRAME;
	else
		hs->frameType = WS_OPENING_FRAME;

	return hs->frameType;
}

void
wsMakeFrame(Websockets* ws, enum wsFrameType frameType, uint8_t*payload, uint64_t payload_length)
{
	/* ensure frameType is not an error frame type  ? */
	assert(frameType < WS_EMPTY_FRAME /* first error frame */);

	/* clear buffer */
	StringBuffer_clear(ws->out);
	size_t frame_header_length = 2;
	uint8_t* frame_buf = (uint8_t*)StringBuffer_value(ws->out);

	frame_buf[0] =  (0x80 | frameType);

	if (payload_length < EXTENDED_PAYLOAD_LENGTH)
	{
		frame_buf[1] = (uint8_t)payload_length;
	}
	else if (payload_length <= EXTENDED_PAYLOAD_MAX)
	{
		frame_buf[1] = EXTENDED_PAYLOAD_LENGTH;
		uint16_t payload_length_net = htons(payload_length);
		memcpy(frame_buf[2], &payload_length_net, sizeof(payload_length_net));
		frame_header_length += sizeof(payload_length_net);
	}
	else
	{
		frame_buf[1] = CONTINUED_EXTENDED_PAYLOAD_LENGTH;
		uint64_t payload_length_net = htobe64(payload_length);
		memcpy(frame_buf[2], &payload_length_net, sizeof(payload_length_net));
		frame_header_length += sizeof(payload_length_net);
	}

	/* copy data */
	// ensure output buffer is large enough
	size_t package_length = frame_header_length + payload_length;
	StringBuffer_make_room(ws->out, 0, package_length + 1);

	/* copy input data */
	memcpy(frame_buf[frame_header_length], payload, payload_length);

	// terminate buffer
	frame_buf[package_length] = '\0';
	ws->out->string->length = package_length + 1;
}

void
Websockets_parse_input_frame(Websockets *ws)
{

	uint8_t inputFrame = StringBuffer_value(ws->in)[0];

	ws->

//	if ((inputFrame[0] & 0x70) != 0x0)      // checks extensions off
//		return WS_ERROR_FRAME;
//	if ((inputFrame[0] & 0x80) != 0x80)     // we haven't continuation frames support
//		return WS_ERROR_FRAME;          			// so, fin flag must be set
		int masked = wsinputFrame[1] & 0x80) != 0x80)     // checks masking bit
//		return WS_ERROR_FRAME;

	uint8_t opcode = StringBuffer_value(ws->in)[0] & 0x0F;

	if (opcode == WS_TEXT_FRAME ||
	    opcode == WS_BINARY_FRAME ||
	    opcode == WS_CLOSING_FRAME ||
	    opcode == WS_PING_FRAME ||
	    opcode == WS_PONG_FRAME
	    )
	{
		enum wsFrameType frameType = opcode;

		uint64_t payload_length = getPayloadLength(ws);

		XFDBG("Frame: opcode:%#x payload:%zu, input:%zu, extra:%u", opcode, payload_length);
		if (payload_length > 0)
		{
//			if (payloadLength < inputLength - 6 - payloadFieldExtraBytes) // 4-maskingKey, 2-header
//				return WS_INCOMPLETE_FRAME;

			uint8_t* maskingKey = &inputFrame[2 + payloadFieldExtraBytes];

//			assert(payloadLength == inputLength - 6 - payloadFieldExtraBytes);

			*dataPtr = &inputFrame[2 + payloadFieldExtraBytes + 4];
			*dataLength = payload_length;

			size_t i;
			for (i = 0; i < *dataLength; i++)
				(*dataPtr)[i] = (*dataPtr)[i] ^ maskingKey[i % 4];
		}
		return frameType;
	}

	return WS_ERROR_FRAME;
}
