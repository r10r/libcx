#include "server_tcp.h"

static void
tcp_server_handler(Server* server, ServerEvent event);

Server*
TCPServer_new(const char* ip, uint16_t port)
{
	Server* server = cx_alloc(sizeof(Server));

	TCPServer_init(server, ip, port);
	return server;
}

void
TCPServer_init(Server* server, const char* ip, uint16_t port)
{
	Server_init(server);
	server->socket = (Socket*)TCPSocket_new(ip, port);
	server->f_server_handler = tcp_server_handler;
}

void
TCPServer_free(Server* server)
{
	TCPSocket_free((TCPSocket*)server->socket);
	Server_free(server);
}

static void
timer_cb(ev_loop* loop, ev_timer* timer, int revents)
{
	UNUSED(loop);
	UNUSED(timer);
	UNUSED(revents);

	XLOG("Hello from the manager");
}

static void
tcp_server_handler(Server* server, ServerEvent event)
{
	TCPSocket* socket = (TCPSocket*)server->socket;

	switch (event)
	{
	case SERVER_START:
	{
		XDBG("server event start");
		// ignore SIGPIPE if on linux
		// TODO check if required because libev already blocks signals
		// TODO check if workers are protected from SIGPIPE signals
#if defined(__linux__)
		signal(SIGPIPE, SIG_IGN);
#endif
		// connect to socket
		// connect to socket
		if (Socket_serve((Socket*)socket) != SOCKET_LISTEN)
		{
			Socket_print_status((Socket*)socket);
			exit(1);
		}

		XFLOG("Listening to socket: %s:%d", socket->ip, socket->port);
		// TODO start worker supervisor
		ev_timer* timer = cx_alloc(sizeof(ev_timer));
		ev_timer_init(timer, timer_cb, 0., 15.);
		ev_timer_again(EV_DEFAULT, timer);
		break;
	}
	case SERVER_SHUTDOWN:
		XDBG("server event stop");
		break;
	}
}
