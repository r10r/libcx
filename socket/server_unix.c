#include "server_unix.h"
#include "worker_unix.h" /* FIXME circular inclusion */

static void
unix_server_handler(Server* server, ServerEvent event);

Server*
UnixServer_new(const char* sock_path)
{
	Server* server = malloc(sizeof(Server));

	UnixServer_init(server, sock_path);
	return server;
}

void
UnixServer_init(Server* server, const char* socket_path)
{
	Server_init(server);
	server->socket = (Socket*)UnixSocket_new(socket_path);
	server->f_server_handler = unix_server_handler;
}

void
UnixServer_free(Server* server)
{
	UnixSocket_free((UnixSocket*)server->socket);
	Server_free(server);
}

static void
supervisor_watcher_cb(ev_loop* loop, ev_timer* timer, int revents)
{
	XDBG("Hello from the supervisor");
}

/* can be replaced by the server */
static void
unix_server_handler(Server* server, ServerEvent event)
{
	UnixSocket* socket = (UnixSocket*)server->socket;

	switch (event)
	{
	case SERVER_START:
	{
		XDBG("server event start");
		// unlink existing socket
		unlink(socket->path);
		// ignore SIGPIPE if on linux
		// TODO check if required because libev already blocks signals
		// TODO check if workers are protected from SIGPIPE signals
#if defined(__linux__)
		signal(SIGPIPE, SIG_IGN);
#endif
		// connect to socket
		if (Socket_serve((Socket*)socket) != SOCKET_LISTEN)
		{
			Socket_print_status((Socket*)socket);
			exit(1);
		}
		// TODO start worker supervisor
		ev_timer* supervisor_watcher = malloc(sizeof(ev_timer));
		ev_timer_init(supervisor_watcher, supervisor_watcher_cb, 0., 15.);
		ev_timer_again(EV_DEFAULT, supervisor_watcher);
		break;
	}
	case SERVER_SHUTDOWN:
		XDBG("server event stop");
		break;
	}
}
