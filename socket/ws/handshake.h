#ifndef CX_WS_HANDSHAKE_H
#define CX_WS_HANDSHAKE_H

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
	handshake->error_message = StringBuffer_from_printf(128, format, __VA_ARGS__)

#define CHECK_PROTOCOL_VALUE(handshake, index, val, ignorecase) \
	if (!Message_protocol_value_equals(message, index, val, ignorecase)) \
	{ \
		PARSE_ERROR(handshake, "invalid protocol value[%d]: expected %s was %s",  \
			    index, val, NULLS(Message_get_protocol_value(message, index))); \
		return -1; \
	}


void
WebsocketsHandshake_reply(WebsocketsHandshake* handshake, StringBuffer* out);

/* return 1 if message was parsed successfully, -1 on error */
int
WebsocketsHandshake_parse(WebsocketsHandshake* handshake, StringBuffer* in);

WebsocketsHandshake*
WebsocketsHandshake_new(void);

void
WebsocketsHandshake_free(WebsocketsHandshake* handshake);

#endif
