#include "ws_connection.h"

//#define WS_BUFFER_LENGTH 0xffff
#define WS_BUFFER_LENGTH 512

#define CXDBG(con, message) \
	XFDBG("Connection[%d] - " message, con->fd)

#define CXFDBG(con, message, ...) \
	XFDBG("Connection[%d] - " message, con->fd, __VA_ARGS__)


Websockets*
Websockets_new()
{
	Websockets* ws = calloc(1, sizeof(Websockets));

	ws->in = StringBuffer_new(WS_BUFFER_LENGTH);
	ws->out = StringBuffer_new(WS_BUFFER_LENGTH);
	ws->state = WS_STATE_NEW;
	return ws;
}

void
Websockets_free(Websockets* ws)
{
	StringBuffer_free(ws->in);
	StringBuffer_free(ws->out);
	free(ws);
}

static int
Websockets_process_handshake(Connection* con, Websockets* ws)
{
	CXDBG(con, "Parse handshake frame");
	WebsocketsHandshake* handshake = WebsocketsHandshake_new();
	int parse_result = WebsocketsHandshake_parse(handshake, ws->in);

	if (parse_result == -1)
	{
		printf("Handshake parse error: %s\n", StringBuffer_value(handshake->error_message));
		WebsocketsHandshake_free(handshake);
		return -1;
	}
	else
	{
		WebsocketsHandshake_reply(handshake, ws->out);
		CXFDBG(con, "sending handshake response: \n%s\n", StringBuffer_value(ws->out));
		Connection_send_buffer(con, ws->out);
		WebsocketsHandshake_free(handshake);
		ws->state = WS_STATE_ESTABLISHED;
		StringBuffer_clear(ws->in);
		StringBuffer_clear(ws->out);
		return 1;
	}
}

int
Websockets_process(Connection* con, Websockets* ws)
{
	CXDBG(con, "Websockets_process.");

	switch (ws->state)
	{
	case WS_STATE_NEW:
		CXFDBG(con, "Process handshake: \n%s\n", StringBuffer_value(ws->in));
		return Websockets_process_handshake(con, ws);
	case WS_STATE_ESTABLISHED:
		CXDBG(con, "Process frame:");
		int res = WebsocketsFrame_parse(ws);
		if (res == 1)
		{
			/* removed processed frame from input */
			size_t nbytes =  (size_t)(ws->frame.payload_raw_last - ws->frame.raw);
			StringBuffer_shift(ws->in, nbytes);
		}
		return res;
	case WS_STATE_CLOSED:
	case WS_STATE_ERROR:
		return -1;
	}

//	CXFDBG(con, "Received frame: type:%#1x state:%d", ws->frameType, ws->state);

//	if (ws->frameType == WS_ERROR_FRAME)
//	{
//		CXDBG(con, "Error in incoming frame");
//
//		if (ws->state == WS_STATE_OPENING)
//		{
//			CXDBG(con, "Bad request");
//			StringBuffer_printf(ws->out, BAD_REQUEST, versionField, version);
//			Connection_send_buffer(con, ws->out);
//			return -1;
//		}
//		else
//		{
//			CXDBG(con, "Send closing frame");
//			Websockets_send_frame(con, ws, WS_CLOSING_FRAME);
//			Websockets_reset(ws);
//			StringBuffer_clear(ws->in);
//			return 1;
//		}
//	}

//	if (ws->state == WS_STATE_OPENING)
//	{
//		if (ws->frameType == WS_OPENING_FRAME)
//		{
//			CXFDBG(con, "Opening frame for resource : %s", ws->handshake.resource);
//			// if resource is right, generate answer handshake and send it
//			if (strcmp(ws->handshake.resource, RESOURCE) == 0)
//			{
//				Websockets_send_handshake(con, ws);
//				Websockets_reset(ws);
//				StringBuffer_clear(ws->in);
//				return 1;
//			}
//			else
//			{
//				CXFDBG(con, "Resource not found : %s", ws->handshake.resource);
//				StringBuffer_cat(ws->out, NOT_FOUND);
//				Connection_send_buffer(con, ws->out);
//				return -1;
//			}
//		}
//		else
//		{
//			CXDBG(con, "Input garbage - not a websockets frame\n");
//			return -1;
//		}
//	}
//
//	/* close connection afterwards */
//	if (ws->frameType == WS_CLOSING_FRAME)
//	{
//		if (ws->state != WS_STATE_CLOSING)
//		{
//			CXDBG(con, "Send closing frame");
//			Websockets_send_frame(con, ws, WS_CLOSING_FRAME);
//		}
//		return 0;
//	}
//
//	if (ws->frameType == WS_TEXT_FRAME)
//	{
//		CXDBG(con, "Reply to incomming text frame");
//		Websockets_send_frame(con, ws, WS_TEXT_FRAME);
//		Websockets_reset(ws);
//		return 1;
//	}

//	return 1;
}
