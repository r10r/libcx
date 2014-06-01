#include "handshake.h"

static const char* const connectionField = "Connection:";
static const char* upgrade = "upgrade";
static const char* upgrade2 = "Upgrade";
static const char* upgradeField = "Upgrade: ";
static const char* websocket = "websocket";
static const char* hostField = "Host: ";
static const char* originField = "Origin: ";
static const char* keyField = "Sec-WebSocket-Key: ";
static const char* protocolField = "Sec-WebSocket-Protocol: ";
static const char* versionField = "Sec-WebSocket-Version: ";
static const char* version = "13";
static const char* WS_SECRET = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

void
WebsocketsHandshake_reply(WebsocketsHandshake *handshake, StringBuffer *out)
{
	/* generate the response key */
	char* responseKey = NULL;
	size_t handshake_key_length = strlen(handshake->key);
	size_t handshake_length = handshake_key_length + sizeof(WS_SECRET);

	responseKey = malloc(handshake_length);

	memcpy(responseKey, handshake->key, handshake_key_length);
	memcpy(&(responseKey[handshake_key_length]), WS_SECRET, sizeof(WS_SECRET));
	char shaHash[20];
	memset(shaHash, 0, sizeof(shaHash));
	sha1(shaHash, responseKey, handshake_length * 8);
	base64enc(responseKey, shaHash, 20);

	StringBuffer_printf(out,
			"HTTP/1.1 101 Switching Protocols\r\n"
			"Upgrade: websocket\r\n"
			"Connection: Upgrade\r\n"
			"Sec-WebSocket-Accept: %s\r\n"
			"\r\n", responseKey);

	free(responseKey);
}

void
WebsocketsHandshake_free(WebsocketsHandshake *handshake)
{
		free(handshake->host);
		free(handshake->origin);
		free(handshake->resource);
		free(handshake->key);
}

static char*
getUptoLinefeed(const char* startFrom)
{
	char* line = NULL;
	size_t line_length = (size_t)(strstr(startFrom, "\r\n") - startFrom);

	line = (char*)malloc(line_length + 1); //+1 for '\x00'

	memcpy(line, startFrom, line_length);
	line[line_length] = '\0';

	return line;
}

void
WebsocketsHandshake_parse(StringBuffer *in)
{
	WebsocketsHandshake handshake;
	memset(&handshake, 0, sizeof(handshake));

	// TODO check input frame
	/*
	 *    if (strstr(str->data, "\r\n\r\n") != NULL || strstr(str->data, "\n\n") != NULL) {
      libwebsock_handshake_finish(bev, state);
    }
	 */

	char* first = strchr(StringBuffer_value(in), ' ');
	first++;
	char* second = strchr(first, ' ');


	handshake.resource = (char*)malloc((size_t)(second - first + 1)); // +1 is for \x00 symbol

	/* use ragel parser | umtp parser to parse the handshake */
}
