#ifndef _CX_WS_CONNECTION_H
#define _CX_WS_CONNECTION_H

#include "base/debug.h"
#include "string/string_buffer.h"
#include "socket/connection.h"

#include "websocket.h"
#include "handshake.h"
#include "frame.h"

Websockets*
Websockets_new(void);

void
Websockets_free(Websockets* ws);

void
Websockets_process(Connection* connection, Websockets* ws);

void
Websockets_process_frame(Connection* connection, Websockets* ws);

void
Connection_send_error(Connection* connection, Websockets* ws, WebsocketsStatusCode code, const char* message);

#define Connection_send_frame(conn, header_bits, payload, nchars) \
	Connection_send_buffer(conn, WebsocketsFrame_create(header_bits, payload, nchars))

#endif
