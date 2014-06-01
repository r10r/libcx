#ifndef CX_WS_HANDSHAKE_H
#define CX_WS_HANDSHAKE_H

#include "websocket.h"
#include "umtp/message_parser.h"

const char* const BAD_REQUEST =
	"HTTP/1.1 400 Bad Request\r\n"
	"%s%s\r\n"
	"\r\n";

const char* const NOT_FOUND =
	"HTTP/1.1 404 Not Found\r\n"
	"\r\n";

#endif
