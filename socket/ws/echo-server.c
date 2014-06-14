#include "base/test.h"
#include "base/base.h"
#include "socket/server_unix.h"
#include "socket/server_tcp.h"
#include "ws_connection.h"

static Connection*
ws_connection_handler(Connection* connection, ConnectionEvent event)
{
	Websockets* ws = (Websockets*)connection->data;

	switch (event)
	{
	case CONNECTION_EVENT_DATA:
	{
		Websockets_process(connection, ws);
		/* state must be established or error / handshake error */
		assert(ws->state != WS_STATE_NEW);

		switch (ws->state)
		{
		case WS_STATE_CLOSE:
		case WS_STATE_ERROR:
		case WS_STATE_ERROR_HANDSHAKE_FAILED:
			Websockets_free(ws);
			Connection_close(connection);
			break;
		default:         /* makes compiler happy */
			break;
		}
		break;
	}
	case CONNECTION_EVENT_CLOSE_READ:
	case CONNECTION_EVENT_ERRNO:
	case CONNECTION_EVENT_ERROR_WRITE:
		XFDBG("Event %d. Closing connection", event);
		Websockets_free(ws);
		Connection_close(connection);
		break;
	}
	return connection;
}

static ssize_t
ws_connection_data_handler(Connection* connection)
{
	Websockets* ws = (Websockets*)connection->data;
	size_t nused_before = StringBuffer_used(ws->in);

	/*
	 * TODO check whether we have an incomplete frame or whether this is a new frame ?
	 * - let the buffer define the read size ?
	 *
	 */
	BufferStatus status;

	if (ws->state == WS_STATE_NEW)
	{
		status = StringBuffer_fdxload(ws->in, connection->fd, WS_HANDSHAKE_BUFFER_SIZE, 0);
		if (status == CX_OK)
		{
			size_t new = StringBuffer_used(ws->in) - nused_before;
			assert(new < SSIZE_MAX); /* application bug */
			return (ssize_t)new;
		}
	}
	else
	{
		status = StringBuffer_ffill(ws->in, connection->fd, 0);
		if (status == CX_OK)
		{
			size_t new = StringBuffer_used(ws->in) - nused_before;
			assert(new < SSIZE_MAX); /* application bug */
			return (ssize_t)new;
		}
	}

	return status;
}

static Connection*
WebsocketsConnection_new()
{
	Connection* connection = Connection_new(NULL, -1);

	connection->f_data_handler = ws_connection_data_handler;
	connection->f_handler = ws_connection_handler;
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
