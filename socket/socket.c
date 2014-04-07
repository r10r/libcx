#include "socket.h"

void
Socket_print_status(Socket* socket)
{
	switch (socket->status)
	{
	// FIXME print socket address (path or IP)
	case SOCKET_INITIALIZED:
		XDBG("Socket initialized\n");
		break;
	case SOCKET_CREATED:
		XDBG("Socket created\n");
		break;
	case SOCKET_BIND:
		XDBG("Socket bind\n");
		break;
	case SOCKET_LISTEN:
		XDBG("Socket listen\n");
		break;
	case SOCKET_ERROR_INVALID_ADDRESS:
		XFERR("Invalid socket address: %s\n", ""); // FIXME print socket address
		break;
	case SOCKET_ERROR_ERRNO:
		XERR("Internal socket error");
		break;
	}
}

SocketStatus
Socket_serve(Socket* socket)
{
	if (socket->status != SOCKET_INITIALIZED)
		return socket->status;

	if (Socket_create(socket) != SOCKET_CREATED)
		return socket->status;

	if (Socket_bind(socket) != SOCKET_BIND)
		return socket->status;

	if (Socket_listen(socket) != SOCKET_LISTEN)
		return socket->status;

#if (!defined (__linux__) && defined(__unix__)) || (defined(__APPLE__) && defined(__MACH__))
	// http://stackoverflow.com/questions/108183/how-to-prevent-sigpipes-or-handle-them-properly
	// http://nadeausoftware.com/articles/2012/01/c_c_tip_how_use_compiler_predefined_macros_detect_operating_system
	// both server and client socket must be protected against SIGPIPE
	Socket_enable_option(socket, SO_NOSIGPIPE);  /* do not send SIGIPIPE on EPIPE */
#endif

	return socket->status;
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
Socket_bind(Socket* socket)
{
	int ret = bind(socket->fd, socket->address, socket->address_size);

	if (ret == 0)
		socket->status = SOCKET_BIND;
	else if (ret == -1)
		socket->status = SOCKET_ERROR_ERRNO;

	return socket->status;
}

SocketStatus
Socket_listen(Socket* socket)
{
	int ret = listen(socket->fd, socket->backlog);

	if (ret == 0)
		socket->status = SOCKET_LISTEN;
	else if (ret == -1)
		socket->status = SOCKET_ERROR_ERRNO;

	return socket->status;
}

void
enable_so_opt(int fd, int option)
{
	int enable = 1;

	setsockopt(fd, SOL_SOCKET, option, (void*)&enable, sizeof(enable));
}