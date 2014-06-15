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

bool
Websockets_process_handshake(Connection* con, Websockets* ws);

bool
Websockets_can_parse(Connection* conn, Websockets* ws);

bool
WebsocketsFrame_parse_length(Websockets* ws);


void
ws_send(Connection* conn, StringBuffer* buf, F_SendFinished* f_finished);

void
ws_send_error(Connection* conn, Websockets* ws, WebsocketsStatusCode status_code, const char* message);

void
ws_send_frame(Connection* conn, uint8_t opcode, const char* payload, size_t nchars, F_SendFinished* f_finished);

bool
WebsocketsFrame_complete(Websockets* ws);

void
ws_close_connection(Connection* conn);

#endif
