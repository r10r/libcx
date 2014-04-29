#ifndef WS_CONNECTION_H
#define WS_CONNECTION_H

#include "websocket.h"
#include "string/string_buffer.h"
#include "socket/connection.h"
#include "base/debug.h"

#define PACKET_DUMP

typedef struct websockets_state_t
{
	StringBuffer* in;
	StringBuffer* out;
	enum wsState state;
	enum wsFrameType frameType;
	struct handshake handshake;
	uint8_t* data;
	size_t dataLength;
} Websockets;

Websockets*
Websockets_new(void);

void
Websockets_free(Websockets* ws);

int
Websockets_process(Connection* connection, Websockets* ws);

#endif
