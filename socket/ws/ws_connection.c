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
	Websockets* ws = cx_alloc(sizeof(Websockets));

	ws->in = StringBuffer_new(WS_BUFFER_LENGTH);
	ws->out = StringBuffer_new(WS_BUFFER_LENGTH);
	ws->error_message = StringBuffer_new(WS_BUFFER_LENGTH);
	ws->state = WS_STATE_NEW;
	return ws;
}

void
Websockets_free(Websockets* ws)
{
	StringBuffer_free(ws->in);
	StringBuffer_free(ws->out);
	StringBuffer_free(ws->error_message);
	cx_free(ws);
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
	WebsocketsFrame_parse(&ws->frame, (uint8_t*)StringBuffer_value(ws->in), StringBuffer_used(ws->in));
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

		assert(ws->frame.length <= UINT32_MAX); /* FIXME refactor String_* to accept uint64_t */
		StringBuffer_shift(ws->in, (uint32_t)ws->frame.length);

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
	else if (ws->state == WS_STATE_ERROR)
	{
		size_t error_message_length = StringBuffer_used(ws->error_message);
		CXFDBG(con, "Sending error message (length %zu): %d %s",
		       error_message_length, ws->status_code, StringBuffer_value(ws->error_message));
		WebsocketsFrame_write_error(ws);
		Connection_send_buffer(con, ws->out);
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
		assert(StringBuffer_used(ws->in) >= 2);
		WebsocketsFrame_parse_header(&ws->frame, StringBuffer_value(ws->in), StringBuffer_used(ws->in));

		// TODO if frame is a control frame fail connection if message is
		// not fully buffered (fragmented control frames are not allowed)

		if (ws->frame.rsv1 || ws->frame.rsv2 || ws->frame.rsv3)
		{
			ws->state = WS_STATE_ERROR;
			Websockets_error(ws, WS_CODE_ERROR_PROTOCOL, "RSV bits must not be set without extension.");
			size_t error_message_length = StringBuffer_used(ws->error_message);
			CXFDBG(con, "Sending error message (length %zu): %d %s",
			       error_message_length, ws->status_code, StringBuffer_value(ws->error_message));
			WebsocketsFrame_write_error(ws);
			Connection_send_buffer(con, ws->out);
		}
		else
		{
			if (WebsocketsFrame_buffer_level(ws) >= 0)
				Websockets_process_frame(con, ws);
			else
				ws->state = WS_STATE_FRAME_INCOMPLETE;
		}
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
