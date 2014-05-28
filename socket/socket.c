#include "socket.h"

void
Socket_print_status(Socket* sock)
{
	switch (sock->status)
	{
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
		XFERRNO("Invalid socket address: %s", ""); // FIXME print socket address
		break;
	case SOCKET_ERROR_ERRNO:
		XERRNO("Internal socket error");
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
	XFDBG("Enable option %d on socket %d", fd, option);
	int enable = 1;

	int result = setsockopt(fd, SOL_SOCKET, option, (void*)&enable, sizeof(enable));

	if (result == 0)
	{
		XFDBG("Enabled socket option %d on socket #%d ", option, fd);
	}
	else
	{
		XFERRNO("Failed to enable socket option %d on socket #%d ", option, fd);
	}
}

/*
 * @param millis timeout in milliseconds (> 0)
 * @param optname SO_RCVTIMEO | SO_SNDTIMEO
 * @param name the string version of SO_RCVTIMEO | SO_SNDTIMEO
 */
void
Socket_set_timeout(Socket* sock, long millis, int optname, const char* name)
{
	if (millis > 0)
	{
		struct timeval timeout;
		TIMEVAL_SET_MILLIS(timeout, millis);

		int result = setsockopt(sock->fd, SOL_SOCKET, optname, (char*)&timeout, sizeof(timeout));

		if (result == 0)
		{
			XFDBG("%s timeout on socket %d: %ld millis", name, sock->fd, millis);
		}
		else
		{
			XFERRNO("Failed to set %s timeout on socket %d to: %ld millis ", name, sock->fd, millis);
		}
	}
	else
	{
		XFDBG("Invalid %s timeout for socket %d: %ld millis", name, sock->fd, millis);
	}
}
