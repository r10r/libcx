#include "ws_connection.h"

//#define WS_BUFFER_LENGTH 0xffff

static void
WebsocketsFrame_process_control_frame(Connection* conn, Websockets* ws);

static void
Websockets_process_frame(Connection* connection, Websockets* ws);

static bool
Websockets_process_handshake(Connection* con, Websockets* ws);

static bool
WebsocketsFrame_parse_length(Websockets* ws);

static void
ws_send(Connection* conn, StringBuffer* buf, F_ResponseFinished* f_send_finished);

static void
ws_send_error(Connection* conn, Websockets* ws, WebsocketsStatusCode status_code, const char* message);

static void
ws_send_frame(Connection* conn, uint8_t opcode, const char* payload, size_t nchars, F_ResponseFinished* f_send_finished);

static bool
WebsocketsFrame_complete(Websockets* ws);

static void
ws_close_connection(Connection* conn);

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
	cx_free(ws->resource);
	StringBuffer_free(ws->in);
	StringBuffer_free(ws->fragments_buffer);
	cx_free(ws);
}

static void
handshake_send_finished(Response* response, void* conn)
{
	UNUSED(response);
	CXFDBG((Connection*)conn, "Handshake was send %p", (void*)response);

	/* accept more data */
	Websockets* ws = (Websockets*)Connection_get_data(conn);
	StringBuffer_clear(ws->in);
	ws->state = WS_STATE_ESTABLISHED;
	((Connection*)conn)->f_receive_enable((Connection*)conn);

	Connection_callback((Connection*)conn, on_start);
}

static void
frame_send_finished(Response* response, void* conn)
{
	UNUSED(response);
	UNUSED(conn);
	CXFDBG((Connection*)conn, "Frame was send %p", (void*)response);
}

static void
frame_send_close(Response* response, void* conn)
{
	UNUSED(response);
	CXFDBG((Connection*)conn, "Frame was send %p", (void*)response);

	ws_close_connection(conn);
}

static void
error_send_finished(Response* response, void* conn)
{
	UNUSED(response);
	CXFDBG((Connection*)conn, "Error was send. Closing connection now %p", (void*)response);

	ws_close_connection((Connection*)conn);
}

static void
ws_send(Connection* conn, StringBuffer* buf, F_ResponseFinished* f_send_finished)
{
	size_t nused = StringBuffer_used(buf);

	if (nused < WS_FRAME_SIZE_MIN)
	{
		CXFERR(conn, "Invalid frame size %zu", nused);
	}
	else
	{
		Response* response = Response_new(buf);
		CXFDBG(conn, "Sending response [%p]", (void*)response);
		conn->f_send(conn, response, f_send_finished);
	}
}

static void
ws_send_error(Connection* conn, Websockets* ws, WebsocketsStatusCode status_code, const char* message)
{
	ws->state = WS_STATE_ERROR;
	conn->f_receive_close(conn);
	CXFWARN(conn, "Send error frame [%d:%s]", status_code, message);
	ws_send(conn, WebsocketsFrame_create_error(status_code, message), error_send_finished);
}

static void
ws_send_frame(Connection* conn, uint8_t opcode, const char* payload, size_t nchars, F_ResponseFinished* f_send_finished)
{
	StringBuffer* out = WebsocketsFrame_create(opcode, payload, nchars);

	ws_send(conn, out, f_send_finished);
}

static void
ws_close_connection(Connection* conn)
{
	Websockets* ws = (Websockets*)Connection_get_data(conn);

	Websockets_free(ws);
	conn->f_close(conn);
}

static bool
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
		ws->resource = cx_strdup(handshake->resource);
		StringBuffer* handshake_buffer = WebsocketsHandshake_create_reply(handshake);
		WebsocketsHandshake_free(handshake);
		CXFDBG(conn, "sending handshake response: \n%s", StringBuffer_value(handshake_buffer));
		ws_send(conn, handshake_buffer, handshake_send_finished);
		return true;
	}
}

/* true if frame length is detected and buffer was resized, false else */
static bool
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

static bool
WebsocketsFrame_complete(Websockets* ws)
{
	return StringBuffer_used(ws->in) >=  ws->frame.length;
}

static size_t
request_get_payload(Request* request, const char** payload_ptr)
{
	WebsocketsFrame* frame = (WebsocketsFrame*)Request_get_data(request);

	*payload_ptr = (const char*)frame->payload_raw;
	return frame->payload_length_extended;
}

static void
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
			{
				Request* request = Request_new(frame);
				request->f_get_payload = request_get_payload;
				conn->callbacks->on_request(conn, request);
			}
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
			conn->f_receive_close(conn); /* do not receive any further data */
			ws->state = WS_STATE_CLOSE;
			uint16_t response_status = WebsocketsFrame_response_status(frame);
			ws_send_frame(conn, WS_FRAME_CLOSE, (char*)&response_status, sizeof(response_status), frame_send_close);
			break;
		}
		case WS_FRAME_PING:
		{
			XDBG("Received WS_FRAME_PING");
			/* send PONG frame with unmasked payload data from PING frame */
			XFDBG("ping frame: payload offset:%hhu length:%llu", frame->payload_offset, frame->payload_length_extended);
			assert(frame->payload_length_extended < SIZE_MAX);
			ws_send_frame(conn, WS_FRAME_PONG, (char*)frame->payload_raw, (size_t)frame->payload_length_extended, frame_send_finished);
			break;
		}
		case WS_FRAME_PONG:
		{
			if (ws->num_pings_without_pong == 0)
			{
				CXDBG(conn, "Ignoring unsolicitated PONG frame.");
			}
			else
			{
				CXFDBG(conn, "Received PONG. Clear unacknowledged PINGS (%u).", ws->num_pings_without_pong);
				ws->num_pings_without_pong = 0;
			}
			break;
		}
		default:
			break;
		}
	}
}

static bool
process_frame(Connection* conn, Websockets* ws)
{
	CXDBG(conn, "Process frame");
#ifdef _CX_DEBUG
	StringBuffer_print_bytes_hex(ws->in, FRAME_HEX_NPRINT, "package bytes");
#endif

	WebsocketsFrame_parse_header(&ws->frame, StringBuffer_value(ws->in));
	if (ws->frame.rsv1 || ws->frame.rsv2 || ws->frame.rsv3)
		ws_send_error(conn, ws, WS_CODE_ERROR_PROTOCOL, "RSV bits must not be set without extension.");
	else
	{
		if (WebsocketsFrame_parse_length(ws))
		{
			if (WebsocketsFrame_complete(ws))
			{
				Websockets_process_frame(conn, ws);
				if (ws->state == WS_STATE_ESTABLISHED)
					return true;
			}
			else
				CXFDBG(conn, "Incomplete frame (length %zu)", StringBuffer_used(ws->in));
		}
	}
	return false;
}

static void
process_frames(Connection* conn, Websockets* ws)
{
	while (process_frame(conn, ws))
	{
		CXDBG(conn, "Shifting input buffer");
		assert(ws->frame.length < SIZE_MAX);
		StringBuffer_shift(ws->in, (size_t)ws->frame.length);

		if (StringBuffer_used(ws->in) < WS_FRAME_LENGTH_MIN)
			break;
	}
}

static void
ws_connection_read(Connection* conn, int fd)
{
	CXDBG(conn, "read data");
	Websockets* ws = (Websockets*)Connection_get_data(conn);

	if (ws->state == WS_STATE_NEW)
	{
		/* receive handshake */
		ws->in = StringBuffer_new(WS_HANDSHAKE_BUFFER_SIZE);
		CXFDBG(conn, "Created input buffer[%p] size %zu", (void*)ws->in, StringBuffer_length(ws->in));
		StringBuffer_fdload(ws->in, fd, WS_HANDSHAKE_BUFFER_SIZE);

		XFDBG("HANDSHAKE ----->\n%s\n------>\n", StringBuffer_value(ws->in));

		/* do not process any incomming data until handshake was send (client must wait nevertheless) */
		conn->f_receive_disable(conn);

		/* if handshake can't be read close connection right away */
		if (!Websockets_process_handshake(conn, ws))
			ws_close_connection(conn);
	}
	else if (ws->state == WS_STATE_CLOSE || ws->state == WS_STATE_ERROR || ws->state == WS_STATE_ERROR_HANDSHAKE_FAILED)
	{
		/* TODO this should never happen because receive watcher
		 * is stopped on error or after receiving a close frame
		 */
		CXFERR(conn, "Unexpected state %d. Input should have been disabled.", ws->state);
		ws_close_connection(conn);
	}
	else
	{
		StringBuffer_ffill(ws->in, fd);

		CXFDBG(conn, "websockets state: %d, buffer status %d", ws->state, ws->in->status);

		switch (ws->in->status)
		{
		case STRING_BUFFER_STATUS_OK:
			process_frames(conn, ws);
			break;
		case STRING_BUFFER_STATUS_EOF:
			/* todo check if there is any data to send */
			XDBG("received EOF - closing connection");
			if (StringBuffer_used(ws->in) >= WS_FRAME_LENGTH_MIN)
				process_frames(conn, ws);       /* process remaining frames */
			// FIXME wait for pending responses to be send pending responses,
			// close after last response has been send (with error / normal close ?)
			ws_close_connection(conn);
			break;
		case STRING_BUFFER_STATUS_ERROR_TO_SMALL:
		case STRING_BUFFER_STATUS_ERROR_INVALID_ACCESS:
		case STRING_BUFFER_STATUS_ERROR_INVALID_READ_SIZE:
		{
			CXFDBG(conn, "buffer error %d : closing connection", ws->in->status);
			conn->f_receive_close(conn);
			ws_send_error(conn, ws, WS_CODE_SERVER_ERROR, "Internal buffer error");
			break;
		}
		case STRING_BUFFER_STATUS_ERROR_ERRNO:
		{
			if (ws->in->error_errno != EWOULDBLOCK)
			{
				CXERRNO(conn, "closing read connection");
				ws_close_connection(conn);
			}
			else
				process_frames(conn, ws);
		}
		}
	}
}

Connection*
WebsocketsConnection_new(ConnectionCallbacks* callbacks)
{
	Connection* conn = Connection_new(callbacks);

	conn->f_receive = ws_connection_read;
	Connection_set_data(conn, Websockets_new());
	return conn;
}
