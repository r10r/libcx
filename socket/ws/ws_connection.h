#ifndef _CX_WS_CONNECTION_H
#define _CX_WS_CONNECTION_H

#include <libcx/string/string_buffer.h>
#include <libcx/socket/connection.h>

#include "websocket.h"
#include "handshake.h"
#include "frame.h"

Websockets*
Websockets_new(void);

void
Websockets_free(Websockets* ws);

Connection*
WebsocketsConnection_new(ConnectionCallbacks* callbacks);

#endif
