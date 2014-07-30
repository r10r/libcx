#include <libcx/socket/server_unix.h>
#include <libcx/socket/server_tcp.h>

#include "ws_connection.h"

static void
print_usage(const char* message) __attribute__((noreturn));

static void
print_usage(const char* message)
{
	fprintf(stderr, "Error: %s\n", message);
	fprintf(stderr, "Usage: $0 <port>\n");
	exit(1);
}

#define MAX_NUM_PINGS 3
#define PING_INTERVAL 15000

static void
on_timeout(Connection* conn)
{
	Websockets* ws = (Websockets*)Connection_get_data(conn);

	UNUSED(ws);

	if (ws->num_pings_without_pong > MAX_NUM_PINGS)
	{
		XFDBG(">> T I M E O U T << (%d unacknowledged pings, interval %d millis)", MAX_NUM_PINGS, PING_INTERVAL);
		conn->f_close(conn);
	}
	else
	{
		StringBuffer* buffer = WebsocketsFrame_create(WS_FRAME_PING, NULL, 0);
		conn->f_send(conn, Response_new(buffer), NULL);
		ws->num_pings_without_pong++;
	}
}

static void
on_start(Connection* conn)
{
	UNUSED(conn);
	CXDBG(conn, "ON START");
	conn->f_timer_start(conn, PING_INTERVAL);
}

static void
on_request(Connection* conn, Request* request)
{
	// FIXME restart the timer here ?
	Websockets* ws = (Websockets*)Connection_get_data(conn);

	UNUSED(ws);

	CXFDBG(conn, "ON REQUEST %s", ws->resource);
	WebsocketsFrame* frame = (WebsocketsFrame*)Request_get_data(request);
	Request_free(request);
	StringBuffer* buffer = WebsocketsFrame_create_echo(frame);
	conn->f_send(conn, Response_new(buffer), NULL);
}

static void
on_error(Connection* conn)
{
	UNUSED(conn);
	CXDBG(conn, "Connection error");
	conn->f_close(conn);
}

static void
on_close(Connection* conn)
{
	UNUSED(conn);
	CXDBG(conn, "ON CLOSE");
}

static ConnectionCallbacks ws_echo_handler = {
	.on_start       = &on_start,
	.on_request     = &on_request,
	.on_error       = &on_error,
	.on_close       = &on_close,
	.on_timeout     = &on_timeout
};

int
main(int argc, char** argv)
{
	if (argc != 2)
		print_usage("Invalid parameter count");

	Server* server = NULL;
	server = (Server*)TCPServer_new("0.0.0.0", (uint16_t)atoi(argv[1]));

	List_push(server->workers, ConnectionWorker_new(WebsocketsConnection_new, &ws_echo_handler));
	List_push(server->workers, ConnectionWorker_new(WebsocketsConnection_new, &ws_echo_handler));
	List_push(server->workers, ConnectionWorker_new(WebsocketsConnection_new, &ws_echo_handler));
	List_push(server->workers, ConnectionWorker_new(WebsocketsConnection_new, &ws_echo_handler));

	Server_start(server);         // blocks
}
