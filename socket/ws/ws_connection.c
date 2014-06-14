#include "ws_connection.h"

//#define WS_BUFFER_LENGTH 0xffff

#define CXDBG(con, message) \
	XFDBG("Connection[%d] - " message, con->fd)

#define CXFDBG(con, message, ...) \
	XFDBG("Connection[%d] - " message, con->fd, __VA_ARGS__)

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
	cx_free(ws);
}

static void
Websockets_process_handshake(Connection* con, Websockets* ws)
{
	WebsocketsHandshake* handshake = WebsocketsHandshake_new();

	WebsocketsHandshake_parse(handshake, ws->in);

	if (handshake->error)
	{
		CXFDBG(con, "Handshake error: %s", StringBuffer_value(handshake->error_message));
		/* TODO send HTTP handshake error 400 */
		ws->state = WS_STATE_ERROR_HANDSHAKE_FAILED;
	}
	else
	{
		StringBuffer* handshake_buffer = WebsocketsHandshake_create_reply(handshake);
		CXFDBG(con, "sending handshake response: \n%s", StringBuffer_value(handshake_buffer));
		Connection_send_buffer(con, handshake_buffer);
		ws->state = WS_STATE_ESTABLISHED;
	}
	WebsocketsHandshake_free(handshake);
}

static int
WebsocketsFrame_parse_length(Websockets* ws)
{
	size_t nused = StringBuffer_used(ws->in);
	size_t nheader_without_masking_key = ws->frame.payload_offset;

	if (ws->frame.masked)
		nheader_without_masking_key -= WS_MASKING_KEY_LENGTH;

	if (nused < nheader_without_masking_key)
	{
		XDBG("Frame to small to parse payload size");
		return 0;
	}
	else
	{
		WebsocketsFrame_parse_payload_length_extended(&ws->frame, StringBuffer_value(ws->in), nused);
		return 1;
	}
}

static int
Websockets_parse_frame(Connection* conn, Websockets* ws)
{
		if (WebsocketsFrame_buffer_level(ws) >= 0)
		{
			Websockets_process_frame(conn, ws);
			return 1;
		}
		return 0;
}

void
Websockets_process(Connection* conn, Websockets* ws)
{
	CXFDBG(conn, "Websockets_process. state:%d", ws->state);

	switch (ws->state)
	{
	case WS_STATE_NEW:
	{
		CXDBG(conn, "Process handshake");
		Websockets_process_handshake(conn, ws);
		break;
	}
	case WS_STATE_ESTABLISHED:
	{
		CXDBG(conn, "Process frame");
		StringBuffer_print_bytes_hex(ws->in, FRAME_HEX_NPRINT, "package bytes");
		size_t nused = StringBuffer_used(ws->in);
		assert(nused >= 2);

		WebsocketsFrame_parse_header(&ws->frame, StringBuffer_value(ws->in), StringBuffer_used(ws->in));

		if (ws->frame.rsv1 || ws->frame.rsv2 || ws->frame.rsv3)
			Connection_send_error(conn, ws, WS_CODE_ERROR_PROTOCOL, "RSV bits must not be set without extension.");
		else
		{
			if (WebsocketsFrame_parse_length(ws))
				if (Websockets_parse_frame(conn, ws))
					ws->state = WS_STATE_ESTABLISHED;
				else
					ws->state = WS_STATE_FRAME_INCOMPLETE;
			else
				ws->state = WS_STATE_FRAME_MISSING_LENGTH;
		}
		break;
	}
	case WS_STATE_FRAME_INCOMPLETE:
		CXDBG(conn, "Incomplete frame");
	{
		CXDBG(conn, "Process incomplete frame");
		if (Websockets_parse_frame(conn, ws))
						ws->state = WS_STATE_ESTABLISHED;
		break;
	}
	case WS_STATE_FRAME_MISSING_LENGTH:
		CXDBG(conn, "Missing frame length");
		if (WebsocketsFrame_parse_length(ws))
		{
			/* grow buffer */
				StringBuffer_make_room(ws->in, 0, ws->frame.length);
				ws->state = WS_STATE_FRAME_INCOMPLETE;
				if (Websockets_parse_frame(conn, ws))
					ws->state = WS_STATE_ESTABLISHED;
				else
			ws->state = WS_STATE_FRAME_INCOMPLETE;
		}
		break;
	default:
		break;
	}
}

void
Websockets_process_frame(Connection* conn, Websockets* ws)
{
	WebsocketsFrame* frame = &ws->frame;

	WebsocketsFrame_parse(frame, (uint8_t*)StringBuffer_value(ws->in), StringBuffer_used(ws->in));

	switch (frame->opcode)
	{
	case WS_FRAME_CONTINUATION:
//		if (ws->last_frament)
//		{
//			// copy last fragment
//		}
//			WebsocketsFrame_unmask_payload_data(&ws->frame);
		break;

	case WS_FRAME_TEXT:
	case WS_FRAME_BINARY:
		/* TODO response might be send async */
		WebsocketsFrame_unmask_payload_data(frame);
		Connection_send_buffer(conn, WebsocketsFrame_create_echo(frame));
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
		Connection_send_error(conn, ws, WS_CODE_ERROR_PROTOCOL, "Invalid opcode");
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

	if (frame->length > WS_CONTROL_MESSAGE_SIZE_MAX)
		Connection_send_error(conn, ws, WS_CODE_ERROR_PROTOCOL, "Invalid control frame (length > 127)");
	else
	{
		switch (frame->opcode)
		{
		case WS_FRAME_CLOSE:
		{
			XFDBG("Received WS_FRAME_CLOSE masked:%u", frame->masked);
			ws->state = WS_STATE_CLOSE;
			uint16_t response_status = WebsocketsFrame_response_status(&ws->frame);
			Connection_send_frame(conn, WS_FRAME_CLOSE, (char*)&response_status, sizeof(response_status));
			break;
		}
		case WS_FRAME_PING:
		{
			XDBG("Received WS_FRAME_PING");
			/* send PONG frame with unmasked payload data from PING frame */
			XFDBG("ping frame: payload offset:%hhu length:%llu", frame->payload_offset, frame->payload_length_extended);
			Connection_send_frame(conn, WS_FRAME_PONG, (char*)frame->payload_raw, frame->payload_length_extended);
			break;
		}
		case WS_FRAME_PONG:
		{
			XERR("Ignoring unsolicitated PONG frame.");
			break;
		}
		default:
			break;
		}
	}
}

void
Connection_send_error(Connection* conn, Websockets* ws, WebsocketsStatusCode status_code, const char* message)
{
	ws->state = WS_STATE_ERROR;
	XFDBG("Connection error %s", message);
	Connection_send_buffer(conn, WebsocketsFrame_create_error(status_code, message));
}
