#include "handshake.h"

static const char* WS_SECRET = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
#define WS_SECRET_LENGTH strlen(WS_SECRET)

StringBuffer*
WebsocketsHandshake_create_reply(WebsocketsHandshake* handshake)
{
	StringBuffer* handshake_buffer = StringBuffer_new(WS_HANDSHAKE_BUFFER_SIZE);
	/* generate the response key */
	char* responseKey = NULL;
	size_t handshake_key_length = strlen(handshake->ws_key);
	size_t handshake_length = WS_SECRET_LENGTH + handshake_key_length;

	responseKey = cx_alloc(handshake_length);

	memcpy(responseKey, handshake->ws_key, handshake_key_length);
	memcpy(&(responseKey[handshake_key_length]), WS_SECRET, WS_SECRET_LENGTH);
	char shaHash[20];
	memset(shaHash, 0, sizeof(shaHash));
	assert(handshake_length < UINT32_MAX / 8);
	sha1(shaHash, responseKey, (uint32_t)(handshake_length * 8));
	base64enc(responseKey, shaHash, 20);

	StringBuffer_printf(handshake_buffer,
			    "HTTP/1.1 101 Switching Protocols\r\n"
			    "Upgrade: websocket\r\n"
			    "Connection: Upgrade\r\n"
			    "Sec-WebSocket-Accept: %s\r\n"
			    "\r\n", responseKey);

	cx_free(responseKey);
	return handshake_buffer;
}

WebsocketsHandshake*
WebsocketsHandshake_new()
{
	WebsocketsHandshake* handshake = cx_alloc(sizeof(WebsocketsHandshake));

	return handshake;
}

void
WebsocketsHandshake_free(WebsocketsHandshake* handshake)
{
	if (handshake)
	{
		if (handshake->error_message)
			StringBuffer_free(handshake->error_message);

		if (handshake->message)
			Message_free(handshake->message);

		cx_free(handshake);
	}
}

/* return 1 if message was parsed successfully, -1 on error */
void
WebsocketsHandshake_parse(WebsocketsHandshake* handshake, StringBuffer* in)
{
	MessageParser* parser = MessageParser_from_buf(in, 1);

	RagelParser_parse((RagelParser*)parser);
	Message* message = parser->message;
	handshake->message = message;

	/* check that all input was parsed */
	size_t nunparsed = RagelParser_unparsed((RagelParser*)parser);
	if (nunparsed > 0)
	{
		Message_free(message);
		MessageParser_free(parser);
		PARSE_ERROR(handshake, "%zu unparsed tokens", nunparsed);
	}
	MessageParser_free(parser);

	/* check if message is a valid request */
	if (message->body != NULL)
	{
		PARSE_ERROR(handshake, "message has a body (length %d)", message->body->length);
	}

	if (message->protocol_values->length != 3)
	{
		PARSE_ERROR(handshake, "invalid protocol line: (%d values, expected 3)", message->protocol_values->length);
	}

	CHECK_PROTOCOL_VALUE(handshake, PROTOCOL_HTTP_VERB, "GET", 0)
	CHECK_PROTOCOL_VALUE(handshake, PROTOCOL_HTTP_VERSION, "HTTP/1.1", 0)

	handshake->resource = Message_get_protocol_value(message, PROTOCOL_HTTP_RESOURCE);

	// TODO distinguish between missing header and invalid value
	if (!Message_header_value_contains(message, "Connection", "Upgrade", 1))
	{
		PARSE_ERROR(handshake, "Invalid header : %s: %s", "Connection", "Upgrade");
	}
	if (!Message_header_value_equals(message, "Upgrade", "websocket", 1))
	{
		PARSE_ERROR(handshake, "Invalid header : %s: %s", "Upgrade", "websocket");
	}
	if (!Message_header_value_equals(message, "Sec-WebSocket-Version", "13", 0))
	{
		PARSE_ERROR(handshake, "Invalid header : %s: %s", "Sec-WebSocket-Version", "13");
	}

	if (!Message_link_header_value(message, "Host", &handshake->host))
	{
		PARSE_ERROR(handshake, "Missing header : %s", "Host");
	}

	// origin field is optional
//	if (!Message_link_header_value(message, "Origin", &handshake->origin))
//	{
//		PARSE_ERROR(handshake, "Missing header : %s", "Origin");
//		return -1;
//	}

	/* TODO check whether the (base64 decoded) nonce is 16 characters long */
	if (!Message_link_header_value(message, "Sec-WebSocket-Key", &handshake->ws_key))
	{
		PARSE_ERROR(handshake, "Missing header : %s", "Sec-WebSocket-Key");
	}
}
