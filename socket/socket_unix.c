#include "socket_unix.h"

UnixSocket*
UnixSocket_new(const char* path)
{
	UnixSocket* unix_sock = cx_alloc(sizeof(UnixSocket));
	Socket* sock = (Socket*)unix_sock;

	sock->namespace = AF_UNIX;
	sock->style = SOCK_STREAM;
	sock->protocol = 0;
	sock->backlog = SOCK_BACKLOG;

	unix_sock->path = cx_strdup(path);

	/* check path length */
	if (strlen(path) <= UNIX_PATH_MAX)
	{
		struct sockaddr_un* address = cx_alloc(sizeof(struct sockaddr_un));
		sock->address = (struct sockaddr*)address;
		sock->address_size = sizeof(struct sockaddr_un);
		address->sun_family = AF_UNIX;
		sprintf(address->sun_path, "%s", path);
		sock->status = SOCKET_INITIALIZED;
	}
	else
		sock->status = SOCKET_ERROR_INVALID_ADDRESS;

	return unix_sock;
}

void
UnixSocket_free(UnixSocket* sock)
{
	cx_free(sock->path);
	cx_free(((Socket*)sock)->address);
	cx_free(sock);
}
