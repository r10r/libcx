#ifndef CX_WS_CONNECTION_H
#define CX_WS_CONNECTION_H

#include <assert.h>

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

#endif
