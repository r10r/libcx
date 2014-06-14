#include "base/test.h"
#include "base/base.h"
#include "socket/server_unix.h"
#include "socket/server_tcp.h"
#include "ws_connection.h"

static void
close_connection(Connection* conn)
{
	Websockets* ws = (Websockets*)conn->data;

	Websockets_free(ws);
	Connection_close(conn);
}

static void
process_frame(Connection* conn, Websockets* ws)
{
	CXDBG(conn, "Process frame");
	StringBuffer_print_bytes_hex(ws->in, FRAME_HEX_NPRINT, "package bytes");
	size_t nused = StringBuffer_used(ws->in);

//	assert(nused >= 2);
	if (nused < 2)
	{
		CXFDBG(conn, "Invalid frame length %zu", nused);
		return;
	}

	WebsocketsFrame_parse_header(&ws->frame, StringBuffer_value(ws->in),
				     StringBuffer_used(ws->in));
	if (ws->frame.rsv1 || ws->frame.rsv2 || ws->frame.rsv3)
		ws_send_error(conn, ws, WS_CODE_ERROR_PROTOCOL,
			      "RSV bits must not be set without extension.");
	else
	{
		if (WebsocketsFrame_parse_length(ws))
		{
			if (WebsocketsFrame_complete(ws))
				Websockets_process_frame(conn, ws);
		}
	}
}

static void
ws_connection_read(Connection* conn)
{
	CXDBG(conn, "read data");
	Websockets* ws = (Websockets*)conn->data;

	if (ws->state == WS_STATE_NEW)
	{
		/* receive handshake */
		ws->in = StringBuffer_new(WS_HANDSHAKE_BUFFER_SIZE);
		StringBuffer_fdload(ws->in, conn->fd, WS_HANDSHAKE_BUFFER_SIZE);
		Websockets_process_handshake(conn, ws);
	}
	else if (ws->state == WS_STATE_CLOSE || ws->state == WS_STATE_ERROR || ws->state == WS_STATE_ERROR_HANDSHAKE_FAILED)
	{
		/* FIXME should not happen because connection should be closed right away */
		Connection_close(conn);
	}
	else
	{
		StringBuffer_ffill(ws->in, conn->fd);

		CXFDBG(conn, "websockets state: %d, buffer status %d", ws->state, ws->in->status);

		switch (ws->in->status)
		{
		case STRING_BUFFER_STATUS_OK:
			process_frame(conn, ws);
			break;
		case STRING_BUFFER_STATUS_EOF:
			/* todo check if there is any data to send */
			XDBG("received EOF - closing connection");
			process_frame(conn, ws);
			close_connection(conn);
			break;
		case STRING_BUFFER_STATUS_ERROR_TO_SMALL:
		case STRING_BUFFER_STATUS_ERROR_INVALID_ACCESS:
		case STRING_BUFFER_STATUS_ERROR_INVALID_READ_SIZE:
		{
			CXFDBG(conn, "closing connection because of error :%d", ws->in->status);
			close_connection(conn);
			break;
		}
		case STRING_BUFFER_STATUS_ERROR_ERRNO:
		{
			if (ws->in->error_errno != EWOULDBLOCK)
			{
				CXERRNO(conn, "closing read connection");
				close_connection(conn);
			}
			else
				process_frame(conn, ws);
		}
		}
	}
}

static void
ws_connection_write(Connection* conn)
{
	Websockets* ws = (Websockets*)conn->data;
	SendUnit* unit = (SendUnit*)List_get(ws->out, 0);

	if (!unit)
	{
		CXDBG(conn, "no more units available for sending");
		Connection_stop_write(conn);
		return;
	}

	CXFDBG(conn, "write data [%p]", unit->buffer);
	size_t ntransmit = StringBuffer_used(unit->buffer) - unit->ntransmitted;
	if (ntransmit == 0)
	{
		CXDBG(conn, "no more data available for writing");
		List_shift(ws->out); /* remove from list */
		unit->f_send_finished(conn, unit);
	}
	else
	{
		ssize_t nwritten = write(conn->fd, StringBuffer_value(unit->buffer), ntransmit);

		if (nwritten == -1)
		{
			// we should not receive EAGAIN here ?
			assert(errno != EAGAIN);
			CXERRNO(conn, "Failed to write data");
			// when writing fails shutdown connection, because buffer is not shifted any longer
			// FIXME free unsend SendUnits
			Connection_close(conn);
		}
		else
		{
			StringBuffer_print_bytes_hex(unit->buffer, 16, "bytes send");
			CXFDBG(conn, "send %zu bytes (%zu remaining)", nwritten, ntransmit - (size_t)nwritten);
			unit->ntransmitted += (size_t)nwritten;
		}
	}
}

static Connection*
WebsocketsConnection_new()
{
	Connection* connection = Connection_new(NULL, -1);

	connection->f_receive_data_handler = ws_connection_read;
	connection->f_send_data_handler = ws_connection_write;
	connection->data = Websockets_new();
	return connection;
}

static UnixWorker*
WebsocketsWorker_new()
{
	UnixWorker* worker = UnixWorker_new();

	worker->f_create_connection = WebsocketsConnection_new;
	return worker;
}

static void
print_usage(const char* message) __attribute__((noreturn));

static void
print_usage(const char* message)
{
	fprintf(stderr, "Error: %s\n", message);
	fprintf(stderr, "Usage: $0 <port>\n");
	exit(1);
}

int
main(int argc, char** argv)
{
	if (argc != 2)
		print_usage("Invalid parameter count");

	Server* server = NULL;

//	if (strcmp(argv[1], "unix") == 0)
//		server = (Server*)UnixServer_new("/tmp/echo.sock");
//	else if (strcmp(argv[1], "tcp") == 0)
	server = (Server*)TCPServer_new("0.0.0.0", (uint16_t)atoi(argv[1]));
//	else
//		print_usage("Invalid server type");

	List_push(server->workers, WebsocketsWorker_new());
	List_push(server->workers, WebsocketsWorker_new());
	List_push(server->workers, WebsocketsWorker_new());
	List_push(server->workers, WebsocketsWorker_new());

	Server_start(server);         // blocks
}
