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
		CXFDBG(con, "Process handshake: \n%s", StringBuffer_value(ws->in));
		return Websockets_process_handshake(con, ws);
	case WS_STATE_ESTABLISHED:
		CXDBG(con, "Process frame:");
		int res = WebsocketsFrame_parse(ws);
		/* send response / error */
		if (StringBuffer_used(ws->out) > 1)
		{
			CXFDBG(con, "Clear response buffer %zu", StringBuffer_used(ws->out));
			Connection_send_buffer(con, ws->out);
			StringBuffer_clear(ws->out);
		}
		if (res == 1)
		{
			/* removed processed frame from input */
			size_t nbytes =  (size_t)(ws->frame.payload_raw_end - ws->frame.raw);
			CXFDBG(con, "Shift input buffer by %zu bytes", nbytes);
			StringBuffer_shift(ws->in, nbytes);
		}
		return res;
	case WS_STATE_CLOSED:
	case WS_STATE_ERROR:
		return -1;
	}
}
