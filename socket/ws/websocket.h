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

#ifndef WEBSOCKET_H
#define WEBSOCKET_H

#include <assert.h>
#include <stdint.h>     /* uint8_t */
#include <stdlib.h>     /* strtoul */
#include <netinet/in.h> /*htons*/
#include <string.h>
#include <stdio.h>      /* sscanf */
#include <ctype.h>      /* isdigit */
//#include <stddef.h> /* size_t */
#include "base64_enc.h"
#include "sha1.h"

typedef enum websockets_opcode_t
{
	WS_FRAME_CONTINUATION = 0x0, /* a continuation frame */
	WS_FRAME_TEXT = 0x1, /* a text frame */
	WS_FRAME_BINARY = 0x2, /* a binary frame */
//	0x3 - 0x7 reserved for further non-control frames
	WS_FRAME_CLOSE = 0x8,	/* a connection close */
	WS_FRAME_PING = 0x9, /* a ping */
	WS_FRAME_PONG = 0xA /* a pong */
// 0xB - 0xF  reserved for further control frames
} WebsocketsOpcode;

typedef struct websockets_header_t
{
	unsigned int fin:1;
	unsigned int rsv1:1;
	unsigned int rsv2:1;
	unsigned int rsv3:1;
	unsigned int opcode:4;
	unsigned int mask:1;
	unsigned int payload_length:1;
} WebsocketsHeader;

typedef struct websockets_frame_t
{
	WebsocketsHeader header;
	WebsocketsOpcode opcode;	/* decoded opcode */
	uint64_t payload_length; 	/* decoded payload length */
	uint32_t mask;						/* contains the mask when masked */

	uint8_t* header_data; 	/* pointer to the start of the frame */
	uint8_t* payload_data; /* pointer to the start of the data (points to input buffer) */
} WebsocketsFrame;

typedef struct websockets_state_t
{
	WebsocketsFrame frame;
	StringBuffer* in;
	StringBuffer* out;
} Websockets;

typedef struct websockets_handshake_t
{
	char* host;
	char* origin;
	char* key;
	char* resource;
	enum wsFrameType frameType;
} WebsocketsHandshake;

#ifndef TRUE
    #define TRUE 1
#endif
#ifndef FALSE
    #define FALSE 0
#endif


wsMakeFrame(Websockets* ws, enum wsFrameType frameType, uint8_t*payload, uint64_t payload_length);

/**
 *
 * @param inputFrame Pointer to input frame. Frame will be modified.
 * @param inputLength Length of input frame
 * @param outDataPtr Return pointer to extracted data in input frame
 * @param outLen Return length of extracted data
 * @return Type of parsed frame
 */
void
wsParseInputFrame(Websockets* ws, enum wsFrameType frameType, uint8_t*payload, uint64_t payload_length);

/**
 * @param hs NULL handshake structure
 */
void
nullHandshake(struct handshake* hs);

/**
 * @param hs free and NULL handshake structure
 */
void
freeHandshake(struct handshake* hs);

#endif  /* WEBSOCKET_H */
