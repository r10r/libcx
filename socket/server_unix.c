#include "server_unix.h"

static void
supervisor_watcher_cb(ev_loop* loop, ev_timer* timer, int revents)
{
	UNUSED(loop);
	UNUSED(timer);
	UNUSED(revents);

	XDBG("Hello from the supervisor");
}

static void
server_start(Server* server)
{
	UnixSocket* socket = (UnixSocket*)server->socket;

	XDBG("server event start");
	/* unlink existing socket */
	unlink(socket->path);
	XFLOG("Using socket: %s", socket->path);
	if (Socket_serve((Socket*)socket) != SOCKET_LISTEN)
	{
		Socket_print_status((Socket*)socket);
		exit(1);
	}

	XFLOG("Listening to socket: %s", socket->path);
	// TODO start worker supervisor
	ev_timer* supervisor_watcher = cx_alloc(sizeof(ev_timer));
	ev_timer_init(supervisor_watcher, supervisor_watcher_cb, 0., 15.);
	ev_timer_again(EV_DEFAULT, supervisor_watcher);
}

static void
server_stop(Server* server)
{
	UNUSED(server);
	XDBG("server event stop");
}

Server*
UnixServer_new(const char* sock_path)
{
	Server* server = cx_alloc(sizeof(Server));

	UnixServer_init(server, sock_path);
	return server;
}

void
UnixServer_init(Server* server, const char* socket_path)
{
	Server_init(server);
	server->socket = (Socket*)UnixSocket_new(socket_path);
	server->f_start = server_start;
	server->f_stop = server_stop;
}

void
UnixServer_free(Server* server)
{
	UnixSocket_free((UnixSocket*)server->socket);
	Server_free(server);
}
