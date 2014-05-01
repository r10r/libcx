#ifndef CX_WS_CONNECTION_H
#define CX_WS_CONNECTION_H

#include "websocket.h"
#include "string/string_buffer.h"
#include "socket/connection.h"
#include "base/debug.h"
#include "handshake.h"

#define PACKET_DUMP

Websockets*
Websockets_new(void);

void
Websockets_free(Websockets* ws);

int
Websockets_process(Connection* connection, Websockets* ws);

#endif
