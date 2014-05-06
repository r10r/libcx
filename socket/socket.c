#include "socket.h"

void
Socket_print_status(Socket* sock)
{
	switch (sock->status)
	{
	// FIXME print socket address (path or IP)
	case SOCKET_INITIALIZED:
		XDBG("Socket initialized");
		break;
	case SOCKET_CREATED:
		XDBG("Socket created");
		break;
	case SOCKET_BIND:
		XDBG("Socket bind");
		break;
	case SOCKET_LISTEN:
		XDBG("Socket listen");
		break;
	case SOCKET_CONNECTED:
		XDBG("Socket connected");
		break;
	case SOCKET_ERROR_INVALID_ADDRESS:
		XFERR("Invalid socket address: %s", ""); // FIXME print socket address
		break;
	case SOCKET_ERROR_ERRNO:
		XERR("Internal socket error");
		break;
	}
}

SocketStatus
Socket_serve(Socket* sock)
{
	if (sock->status != SOCKET_INITIALIZED)
		return sock->status;

	if (Socket_create(sock) != SOCKET_CREATED)
		return sock->status;

#if (!defined (__linux__) && defined(__unix__)) || (defined(__APPLE__) && defined(__MACH__))
	// http://stackoverflow.com/questions/108183/how-to-prevent-sigpipes-or-handle-them-properly
	// http://nadeausoftware.com/articles/2012/01/c_c_tip_how_use_compiler_predefined_macros_detect_operating_system
	// both server and client socket must be protected against SIGPIPE
	Socket_enable_option(sock, SO_NOSIGPIPE);       /* do not send SIGIPIPE on EPIPE */
#endif

	Socket_enable_option(sock, SO_REUSEADDR);       /* avoid address in use after termination */

	if (Socket_bind(sock) != SOCKET_BIND)
		return sock->status;

	if (Socket_listen(sock) != SOCKET_LISTEN)
		return sock->status;

	return sock->status;
}

SocketStatus
Socket_use(Socket* sock)
{
	if (sock->status != SOCKET_INITIALIZED)
		return sock->status;

	if (Socket_create(sock) != SOCKET_CREATED)
		return sock->status;

#if (!defined (__linux__) && defined(__unix__)) || (defined(__APPLE__) && defined(__MACH__))
	// http://stackoverflow.com/questions/108183/how-to-prevent-sigpipes-or-handle-them-properly
	// http://nadeausoftware.com/articles/2012/01/c_c_tip_how_use_compiler_predefined_macros_detect_operating_system
	// both server and client socket must be protected against SIGPIPE
	Socket_enable_option(sock, SO_NOSIGPIPE);       /* do not send SIGIPIPE on EPIPE */
#endif

	Socket_enable_option(sock, SO_REUSEADDR);       /* avoid address in use after termination */

	if (Socket_connect(sock) != SOCKET_CONNECTED)
		return sock->status;

	return sock->status;
}

SocketStatus
Socket_connect(Socket* sock)
{
	int ret = connect(sock->fd, sock->address, sock->address_size);

	if (ret == 0)
		sock->status = SOCKET_CONNECTED;
	else
		sock->status = SOCKET_ERROR_ERRNO;

	return sock->status;
}

SocketStatus
Socket_create(Socket* sock)
{
	sock->fd = socket(sock->namespace, sock->style, sock->protocol);

	if (sock->fd == -1)
		sock->status = SOCKET_ERROR_ERRNO;
	else
		sock->status = SOCKET_CREATED;

	return sock->status;
}

SocketStatus
Socket_bind(Socket* sock)
{
	int ret = bind(sock->fd, sock->address, sock->address_size);

	if (ret == 0)
		sock->status = SOCKET_BIND;
	else if (ret == -1)
		sock->status = SOCKET_ERROR_ERRNO;

	return sock->status;
}

SocketStatus
Socket_listen(Socket* sock)
{
	int ret = listen(sock->fd, sock->backlog);

	if (ret == 0)
		sock->status = SOCKET_LISTEN;
	else if (ret == -1)
		sock->status = SOCKET_ERROR_ERRNO;

	return sock->status;
}

void
enable_so_opt(int fd, int option)
{
	int enable = 1;

	setsockopt(fd, SOL_SOCKET, option, (void*)&enable, sizeof(enable));
}
