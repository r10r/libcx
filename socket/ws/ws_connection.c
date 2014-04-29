#include "ws_connection.h"

static const char* const BAD_REQUEST =
	"HTTP/1.1 400 Bad Request\r\n"
	"%s%s\r\n"
	"\r\n";

static const char* const NOT_FOUND =
	"HTTP/1.1 404 Not Found\r\n"
	"\r\n";

static const char* RESOURCE = "/echo";

#define WS_BUFFER_LENGTH 0xffff

#define CXDBG(con, message) \
	XFDBG("Connection[%d] - " message, con->fd)

#define CXFDBG(con, message, ...) \
	XFDBG("Connection[%d] - " message, con->fd, __VA_ARGS__)


Websockets*
Websockets_new()
{
	Websockets* ws = calloc(1, sizeof(Websockets));

	ws->state = WS_STATE_OPENING;
	ws->frameType = WS_INCOMPLETE_FRAME;
	nullHandshake(&ws->handshake);
	ws->in = StringBuffer_new(WS_BUFFER_LENGTH);
	ws->out = StringBuffer_new(WS_BUFFER_LENGTH);
	return ws;
}

void
Websockets_free(Websockets* ws)
{
	freeHandshake(&ws->handshake);
	StringBuffer_free(ws->in);
	StringBuffer_free(ws->out);
	free(ws);
}

static void
Websockets_reset(Websockets* ws)
{
//	StringBuffer_clear(ws->in);
	StringBuffer_clear(ws->out);
	ws->frameType = WS_INCOMPLETE_FRAME;
}

/* TODO check if this is valid */
#define WS_HEADER_SIZE 4

static void
Websockets_send_frame(Connection* con, Websockets* ws, enum wsFrameType type)
{
	StringBuffer_clear(ws->out);
	size_t frameSize = StringBuffer_used(ws->in) - 1; /* without \0 */
	wsMakeFrame(ws->data, ws->dataLength, (const uint8_t*)(StringBuffer_value(ws->out)), &frameSize, type);
	ws->out->string->length = frameSize;
	Connection_send_buffer(con, ws->out);
	CXFDBG(con, "Send frame --> %zu.", frameSize);
	StringBuffer_shift(ws->in, frameSize + WS_HEADER_SIZE);
}

static void
Websockets_send_handshake(Connection* con, Websockets* ws)
{
	size_t frameSize;

	frameSize = StringBuffer_used(ws->in) - 1;
	StringBuffer_clear(ws->out);
	wsGetHandshakeAnswer(&ws->handshake, (const uint8_t*)(StringBuffer_value(ws->out)), &frameSize);
	ws->out->string->length = frameSize;
	ws->state = WS_STATE_NORMAL;
	Connection_send_buffer(con, ws->out);
}

static void
Websockets_parse_handshake(Websockets* ws)
{
	ws->frameType = wsParseHandshake((const uint8_t*)(StringBuffer_value(ws->in)),
					 StringBuffer_used(ws->in) - 1 /* exclude \0 */, &ws->handshake);
}

static void
Websockets_parse_input_frame(Websockets* ws)
{
	ws->frameType = wsParseInputFrame((const uint8_t*)(StringBuffer_value(ws->in)),
					  StringBuffer_used(ws->in) - 1 /* exclude \0 */, &ws->data, &ws->dataLength);
}

int
Websockets_process(Connection* con, Websockets* ws)
{
	CXDBG(con, "Websockets_process.");
	StringBuffer_log(ws->in, "Input buffer");
	StringBuffer_log(ws->out, "Output buffer");

	/* parse input */
	if (ws->state == WS_STATE_OPENING)
	{
		CXDBG(con, "Parse handshake frame");
		Websockets_parse_handshake(ws);
	}
	else
	{
		CXDBG(con, "Parse input frame");
		Websockets_parse_input_frame(ws);
		CXFDBG(con, "Frame data length %zu", ws->dataLength);
	}

	CXFDBG(con, "Received frame: type:%#1x state:%d", ws->frameType, ws->state);

	if (ws->frameType == WS_ERROR_FRAME)
	{
		CXDBG(con, "Error in incoming frame");

		if (ws->state == WS_STATE_OPENING)
		{
			CXDBG(con, "Bad request");
			StringBuffer_printf(ws->out, BAD_REQUEST, versionField, version);
			Connection_send_buffer(con, ws->out);
			return -1;
		}
		else
		{
			CXDBG(con, "Send closing frame");
			Websockets_send_frame(con, ws, WS_CLOSING_FRAME);
			Websockets_reset(ws);
			StringBuffer_clear(ws->in);
			return 1;
		}
	}

	if (ws->state == WS_STATE_OPENING)
	{
		if (ws->frameType == WS_OPENING_FRAME)
		{
			CXFDBG(con, "Opening frame for resource : %s", ws->handshake.resource);
			// if resource is right, generate answer handshake and send it
			if (strcmp(ws->handshake.resource, RESOURCE) == 0)
			{
				Websockets_send_handshake(con, ws);
				Websockets_reset(ws);
				StringBuffer_clear(ws->in);
				return 1;
			}
			else
			{
				CXFDBG(con, "Resource not found : %s", ws->handshake.resource);
				StringBuffer_cat(ws->out, NOT_FOUND);
				Connection_send_buffer(con, ws->out);
				return -1;
			}
		}
		else
		{
			CXDBG(con, "Input garbage - not a websockets frame\n");
			return -1;
		}
	}

	/* close connection afterwards */
	if (ws->frameType == WS_CLOSING_FRAME)
	{
		if (ws->state != WS_STATE_CLOSING)
		{
			CXDBG(con, "Send closing frame");
			Websockets_send_frame(con, ws, WS_CLOSING_FRAME);
		}
		return 0;
	}

	if (ws->frameType == WS_TEXT_FRAME)
	{
		CXDBG(con, "Reply to incomming text frame");
		Websockets_send_frame(con, ws, WS_TEXT_FRAME);
		Websockets_reset(ws);
		return 1;
	}

	return 1;
}
