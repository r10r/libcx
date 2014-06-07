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
		CXFDBG(con, "Handshake parse error: %s", StringBuffer_value(handshake->error_message));
		WebsocketsHandshake_free(handshake);
		return -1;
	}
	else
	{
		WebsocketsHandshake_reply(handshake, ws->out);
		CXFDBG(con, "sending handshake response: \n%s", StringBuffer_value(ws->out));
		Connection_send_buffer(con, ws->out);
		WebsocketsHandshake_free(handshake);
		StringBuffer_clear(ws->in);
		StringBuffer_clear(ws->out);
		return 1;
	}
}

static void
Websockets_process_frame(Connection* con, Websockets* ws)
{
	WebsocketsFrame_parse(ws);
	WebsocketsFrame_process(ws);

	/* send response | error */
	if (ws->state == WS_STATE_FRAME_SEND_RESPONSE)
	{
		CXFDBG(con, "Sending response buffer (%zu bytes)",
		       StringBuffer_used(ws->out));
		StringBuffer_print_bytes_hex(ws->out, FRAME_HEX_NPRINT, "output message");
		Connection_send_buffer(con, ws->out);

		/* remove processed frame from input */
		CXFDBG(con, "Shift input buffer by %llu bytes", ws->frame.length);
		StringBuffer_shift(ws->in, ws->frame.length);

		StringBuffer_clear(ws->out);

		ws->state = WS_STATE_FRAME_NEW;
	}
	else if (ws->state == WS_STATE_CLOSE)
	{
		CXFDBG(con, "Sending response buffer (%zu bytes)",
		       StringBuffer_used(ws->out));
		StringBuffer_print_bytes_hex(ws->out, FRAME_HEX_NPRINT, "output message");
		Connection_send_buffer(con, ws->out);
		StringBuffer_clear(ws->out);
	}
}

void
Websockets_process(Connection* con, Websockets* ws)
{
	CXFDBG(con, "Websockets_process. state:%d", ws->state);

	switch (ws->state)
	{
	case WS_STATE_NEW:
	{
		CXFDBG(con, "Process handshake: \n%s", StringBuffer_value(ws->in));
		int processed = Websockets_process_handshake(con, ws);
		if (processed == 1)
			ws->state = WS_STATE_FRAME_NEW;
		else
			/* TODO send 404 bad request */
			ws->state = WS_STATE_ERROR;
		break;
	}
	case WS_STATE_FRAME_NEW:
	{
		CXDBG(con, "Process frame");
		StringBuffer_print_bytes_hex(ws->in, FRAME_HEX_NPRINT, "package bytes");
		WebsocketsFrame_parse_header(ws);

		// TODO if frame is a control frame fail connection if message is
		// not fully buffered (fragmented control frames are not allowed)

		if (WebsocketsFrame_buffer_level(ws) >= 0)
			Websockets_process_frame(con, ws);
		else
			ws->state = WS_STATE_FRAME_INCOMPLETE;
	}
	break;
	case WS_STATE_FRAME_INCOMPLETE:
	{
		CXDBG(con, "Process incomplete frame");
		if (WebsocketsFrame_buffer_level(ws) >= 0)
			Websockets_process_frame(con, ws);
	}
	break;
	default:
		break;
	}
}
