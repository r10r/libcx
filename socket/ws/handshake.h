#ifndef _CX_WS_HANDSHAKE_H
#define _CX_WS_HANDSHAKE_H

#include "websocket.h"
#include "umtp/message_parser.h"

static const char* const BAD_REQUEST =
	"HTTP/1.1 400 Bad Request\r\n"
	"%s%s\r\n"
	"\r\n";

static const char* const NOT_FOUND =
	"HTTP/1.1 404 Not Found\r\n"
	"\r\n";

#define PROTOCOL_HTTP_VERB 0
#define PROTOCOL_HTTP_RESOURCE 1
#define PROTOCOL_HTTP_VERSION 2

#define WS_KEY_LENGTH 16

#define PARSE_ERROR(handshake, format, ...) \
	handshake->error = 1; \
	handshake->error_message = StringBuffer_from_printf(128, format, __VA_ARGS__); \
	return

#define CHECK_PROTOCOL_VALUE(handshake, index, val, ignorecase) \
	if (!Message_protocol_value_equals(message, index, val, ignorecase)) \
	{ \
		PARSE_ERROR(handshake, "invalid protocol value[%d]: expected %s was %s",  \
			    index, val, NULLS(Message_get_protocol_value(message, index))); \
	}

StringBuffer*
WebsocketsHandshake_create_reply(WebsocketsHandshake* handshake);

void
WebsocketsHandshake_parse(WebsocketsHandshake* handshake, StringBuffer* in);

WebsocketsHandshake*
WebsocketsHandshake_new(void);

void
WebsocketsHandshake_free(WebsocketsHandshake* handshake);

#endif
