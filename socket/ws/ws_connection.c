#include "ws_connection.h"

//#define WS_BUFFER_LENGTH 0xffff

static void
WebsocketsFrame_process_control_frame(Connection* conn, Websockets* ws);

Websockets*
Websockets_new()
{
	Websockets* ws = cx_alloc(sizeof(Websockets));

	ws->state = WS_STATE_NEW;
	return ws;
}

void
Websockets_free(Websockets* ws)
{
	StringBuffer_free(ws->in);
	StringBuffer_free(ws->fragments_buffer);
	cx_free(ws);
}

static void
handshake_send_finished(Connection* conn, SendBuffer* send_buffer)
{
	UNUSED(send_buffer);
	CXFDBG(conn, "Handshake was send %p", (void*)send_buffer->buffer);

	/* accept more data */
	Websockets* ws = (Websockets*)conn->data;
	StringBuffer_clear(ws->in);
	ws->state = WS_STATE_ESTABLISHED;
	ev_io* watcher = &conn->send_data_watcher;
	ev_set_priority(watcher, 0);
	Connection_start_read(conn); /* process more data */
}

static void
frame_send_finished(Connection* conn, SendBuffer* send_buffer)
{
	UNUSED(send_buffer);
	CXFDBG(conn, "Frame was send %p", (void*)send_buffer->buffer);
	ev_io* watcher = &conn->send_data_watcher;
	ev_set_priority(watcher, 0);
}

static void
frame_send_close(Connection* conn, SendBuffer* send_buffer)
{
	UNUSED(send_buffer);
	CXFDBG(conn, "Frame was send %p", (void*)send_buffer->buffer);

	ws_close_connection(conn);
}

static void
error_send_finished(Connection* conn, SendBuffer* send_buffer)
{
	UNUSED(send_buffer);
	CXFDBG(conn, "Error was send. Closing connection now %p", (void*)send_buffer->buffer);

	ws_close_connection(conn);
}

void
ws_send(Connection* conn, StringBuffer* buf, F_SendFinished* f_finished)
{
	size_t nused = StringBuffer_used(buf);

	if (nused < WS_FRAME_SIZE_MIN)
		CXFERR(conn, "Invalid frame size %zu", nused);
	else
	{
		CXFDBG(conn, "Send frame [%p]", (void*)buf);
		Connection_send(conn, buf, f_finished);
	}
}

void
ws_send_error(Connection* conn, Websockets* ws, WebsocketsStatusCode status_code, const char* message)
{
	ws->state = WS_STATE_ERROR;
	CXFWARN(conn, "Send error frame [%d:%s]", status_code, message);
	ev_io* watcher = &conn->send_data_watcher;
	ev_set_priority(watcher, EV_MAXPRI);
	ws_send(conn, WebsocketsFrame_create_error(status_code, message), error_send_finished);
}

void
ws_send_frame(Connection* conn, uint8_t opcode, const char* payload, size_t nchars, F_SendFinished* f_finished)
{
	StringBuffer* out = WebsocketsFrame_create(opcode, payload, nchars);

	ws_send(conn, out, f_finished);
}

void
ws_close_connection(Connection* conn)
{
	Websockets* ws = (Websockets*)conn->data;

	Websockets_free(ws);
	Connection_close(conn);
}

bool
Websockets_process_handshake(Connection* conn, Websockets* ws)
{
	WebsocketsHandshake* handshake = WebsocketsHandshake_new();

	WebsocketsHandshake_parse(handshake, ws->in);

	if (handshake->error)
	{
		CXFDBG(conn, "Handshake error: %s", StringBuffer_value(handshake->error_message));
		/* TODO send HTTP handshake error 400 */
		ws->state = WS_STATE_ERROR_HANDSHAKE_FAILED;
		WebsocketsHandshake_free(handshake);
		return false;
	}
	else
	{
		StringBuffer* handshake_buffer = WebsocketsHandshake_create_reply(handshake);
		WebsocketsHandshake_free(handshake);
		CXFDBG(conn, "sending handshake response: \n%s", StringBuffer_value(handshake_buffer));
		ev_io* watcher = &conn->send_data_watcher;
		ev_set_priority(watcher, EV_MAXPRI);
		ws_send(conn, handshake_buffer, handshake_send_finished);
		return true;
	}
}

/* true if frame length is detected and buffer was resized, false else */
bool
WebsocketsFrame_parse_length(Websockets* ws)
{
	size_t nused = StringBuffer_used(ws->in);
	size_t nheader_without_masking_key = ws->frame.payload_offset;

	if (ws->frame.masked)
		nheader_without_masking_key -= WS_MASKING_KEY_LENGTH;

	if (nused < nheader_without_masking_key)
	{
		XDBG("Frame to small to parse payload size");
		return false;
	}
	else
	{
		WebsocketsFrame_parse_payload_length_extended(&ws->frame, StringBuffer_value(ws->in));
		if (StringBuffer_length(ws->in) >= ws->frame.length)
		{
			XDBG("Buffer is large enough for frame");
			return true;
		}
		else
		{
			assert(ws->frame.length < SIZE_MAX);
			return StringBuffer_make_room(ws->in, 0, (size_t)ws->frame.length);
		}
	}
}

bool
WebsocketsFrame_complete(Websockets* ws)
{
	return StringBuffer_used(ws->in) >=  ws->frame.length;
}

void
Websockets_process_frame(Connection* conn, Websockets* ws)
{
	WebsocketsFrame* frame = &ws->frame;

	CXFDBG(conn, "Process frame opcode 0x%x", frame->opcode);

	WebsocketsFrame_parse(frame, (uint8_t*)StringBuffer_value(ws->in));

	switch (frame->opcode)
	{
	case WS_FRAME_CONTINUATION:
		/* check whether the previous frame has the fin bit not set */
		if (ws->fragments_buffer)
		{
			WebsocketsFrame_unmask_payload_data(frame);
			assert(frame->payload_length_extended < SIZE_MAX);
			StringBuffer_ncat(ws->fragments_buffer, (char*)frame->payload_raw, (size_t)frame->payload_length_extended);
			if (frame->fin)
			{
				CXFDBG(conn, "Finishing continuation data in buffer[%p]", (void*)ws->fragments_buffer);
				StringBuffer* frag_frame = WebsocketsFrame_create(ws->fragments_opcode,
										  StringBuffer_value(ws->fragments_buffer), StringBuffer_used(ws->fragments_buffer));
				ws_send(conn, frag_frame, frame_send_finished);
				StringBuffer_free(ws->fragments_buffer);
				ws->fragments_buffer = NULL;
			}
			else
				CXFDBG(conn, "Appending continuation data to buffer[%p]", (void*)ws->fragments_buffer);
		}
		else
			ws_send_error(conn, ws, WS_CODE_ERROR_PROTOCOL, "Nothing to continue");
		break;
	case WS_FRAME_TEXT:
	case WS_FRAME_BINARY:

		WebsocketsFrame_unmask_payload_data(frame);
		if (frame->fin)
		{
			if (ws->fragments_buffer)
				ws_send_error(conn, ws, WS_CODE_ERROR_PROTOCOL, "Fragmented frame not finished");
			else
				ws_send(conn, WebsocketsFrame_create_echo(frame), frame_send_finished);
		}
		else
		{
			ws->fragments_buffer = StringBuffer_new(WS_BUFFER_SIZE);         /* TODO leave room for next frames */
			CXFDBG(conn, "Starting continuation data [%p]", (void*)ws->fragments_buffer);
			assert(frame->payload_length_extended < SIZE_MAX);
			StringBuffer_ncat(ws->fragments_buffer, (char*)frame->payload_raw, (size_t)frame->payload_length_extended);
			ws->fragments_opcode = frame->opcode;
		}
		ws->state = WS_STATE_ESTABLISHED;
		break;
	case WS_FRAME_CLOSE:
	case WS_FRAME_PING:
	case WS_FRAME_PONG:
		WebsocketsFrame_unmask_payload_data(frame);
		WebsocketsFrame_process_control_frame(conn, ws);
		ws->state = WS_STATE_ESTABLISHED;
		break;
	default:
		ws_send_error(conn, ws, WS_CODE_ERROR_PROTOCOL, "Invalid opcode");
	}
}

static void
WebsocketsFrame_process_control_frame(Connection* conn, Websockets* ws)
{
	/*
	   All control frames MUST have a payload length of 125 bytes or less
	   and MUST NOT be fragmented.
	 */
	WebsocketsFrame* frame = &ws->frame;

	CXFDBG(conn, "Process control frame 0x%x", frame->opcode);

	if (frame->length > WS_CONTROL_MESSAGE_SIZE_MAX)
		ws_send_error(conn, ws, WS_CODE_ERROR_PROTOCOL, "Invalid control frame (length > 127)");
	else if (frame->fin == 0)
		ws_send_error(conn, ws, WS_CODE_ERROR_PROTOCOL, "Control frames must not be fragmented (FIN bit is set)");
	else
	{
		switch (frame->opcode)
		{
		case WS_FRAME_CLOSE:
		{
			XFDBG("Received WS_FRAME_CLOSE masked:%u", frame->masked);
			ws->state = WS_STATE_CLOSE;
			uint16_t response_status = WebsocketsFrame_response_status(frame);
			ev_io* watcher = &conn->send_data_watcher;
			ev_set_priority(watcher, EV_MAXPRI);
			ws_send_frame(conn, WS_FRAME_CLOSE, (char*)&response_status, sizeof(response_status), frame_send_close);
			break;
		}
		case WS_FRAME_PING:
		{
			XDBG("Received WS_FRAME_PING");
			/* send PONG frame with unmasked payload data from PING frame */
			XFDBG("ping frame: payload offset:%hhu length:%llu", frame->payload_offset, frame->payload_length_extended);
			ev_io* watcher = &conn->send_data_watcher;
			ev_set_priority(watcher, EV_MAXPRI);
			assert(frame->payload_length_extended < SIZE_MAX);
			ws_send_frame(conn, WS_FRAME_PONG, (char*)frame->payload_raw, (size_t)frame->payload_length_extended, frame_send_finished);
			break;
		}
		case WS_FRAME_PONG:
		{
			CXWARN(conn, "Ignoring unsolicitated PONG frame.");
			break;
		}
		default:
			break;
		}
	}
}
