#include "unix_server.h"
#include "unix_worker.h" /* FIXME circular inclusion */

static void
unix_server_handler(Server *server, ServerEvent event);

UnixServer*
UnixServer_new(const char *sock_path)
{
	UnixServer *server = malloc(sizeof(UnixServer));

	UnixServer_init(server, sock_path);
	return server;
}

void
UnixServer_init(UnixServer *unix_server, const char *sock_path)
{
	unix_server->socket_path = sock_path;
	Server *server = (Server*)unix_server;
	Server_init(server);

	server->f_server_handler = unix_server_handler;
}

void
UnixServer_free(UnixServer *server)
{
	free(server);
}

static void
supervisor_watcher_cb(ev_loop *loop, ev_timer *timer, int revents)
{
	printf("Hello from the supervisor\n");
}

/* can be replaced by the server */
static void
unix_server_handler(Server *server, ServerEvent event)
{
	UnixServer *unix_server = (UnixServer*)server;

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
		unix_server->fd = unix_socket_connect(unix_server->socket_path);
		// TODO start worker supervisor
		ev_timer *supervisor_watcher = malloc(sizeof(ev_timer));
		ev_timer_init(supervisor_watcher, supervisor_watcher_cb, 0., 15.);
		ev_timer_again(EV_DEFAULT, supervisor_watcher);
		break;
	}
	case SERVER_SHUTDOWN:
		XDBG("server event stop");
		break;
	}
}

/* FIXME merge with TCP socket connect in socket.c */
int
unix_socket_connect(const char *sock_path)
{
	struct sockaddr_un address;
	int fd = SOCKET_CONNECT_FAILED;
	size_t sock_path_len;
	int ret;
	socklen_t address_size;

	sock_path_len = strlen(sock_path);
	address_size = sizeof(address);
	XFLOG("Starting on socket : [%s]\n", sock_path);

	/* check preconditions */
	if (sock_path_len > UNIX_PATH_MAX)
	{
		fprintf(stderr,
			"Socket path to long (%ld). Path length is limited to %d tokens.\n",
			sock_path_len, UNIX_PATH_MAX);
		return SOCKET_CONNECT_FAILED;
	}

	/* create socket */
	fd = socket(PF_UNIX, SOCK_STREAM, 0);
	if (fd == SOCKET_CREATE_ERROR)
	{
		XERR("Failed to create socket");
		return SOCKET_CONNECT_FAILED;
	}

	/* start with a clean address structure */
	memset(&address, 0, sizeof(struct sockaddr_un));
	address.sun_family = PF_UNIX;
	sprintf(address.sun_path, "%s", sock_path);

	/* bind */
	unlink(sock_path);
	ret = bind(fd, (struct sockaddr *)&address, address_size);
	if (ret != BIND_SUCCESS)
	{
		XERR("Failed to bind to socket");
		return SOCKET_CONNECT_FAILED;
	}

	/* listen */
	ret = listen(fd, SOCK_BACKLOG);
	if (ret != LISTEN_SUCCESS)
	{
		XERR("Failed to listen to socket");
		return SOCKET_CONNECT_FAILED;
	}
	XFLOG("Connected to: fd:%d [%s]\n", fd, sock_path);

#if (!defined (__linux__) && defined(__unix__)) || (defined(__APPLE__) && defined(__MACH__))
	// http://stackoverflow.com/questions/108183/how-to-prevent-sigpipes-or-handle-them-properly
	// http://nadeausoftware.com/articles/2012/01/c_c_tip_how_use_compiler_predefined_macros_detect_operating_system
	// both server and client socket must be protected against SIGPIPE
	enable_so_opt(fd, SO_NOSIGPIPE); /* do not send SIGIPIPE on EPIPE */
#endif

	return fd;
}

void
enable_so_opt(int fd, int option)
{
	int enable = 1;

	setsockopt(fd, SOL_SOCKET, option, (void*)&enable, sizeof(enable));
}
