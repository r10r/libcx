#include "socket_unix.h"

UnixSocket*
UnixSocket_new(const char* path)
{
	UnixSocket* unix_sock = calloc(1, sizeof(UnixSocket));
	Socket* sock = (Socket*)unix_sock;

	sock->namespace = PF_LOCAL; /* == PF_UNIX */
	sock->style = SOCK_STREAM;
	sock->protocol = 0;
	sock->backlog = SOCK_BACKLOG;

	unix_sock->path = strdup(path); // TODO free

	/* check path length */
	if (strlen(path) <= UNIX_PATH_MAX)
	{
		struct sockaddr_un* address = calloc(1, sizeof(struct sockaddr_un));
		sock->address = (struct sockaddr*)address;
		sock->address_size = sizeof(struct sockaddr_un);
		address->sun_family = PF_LOCAL;
		sprintf(address->sun_path, "%s", path);
		sock->status = SOCKET_INITIALIZED;
	}
	else
		sock->status = SOCKET_ERROR_INVALID_ADDRESS; // TODO set socket->error_message

	return unix_sock;
}

void
UnixSocket_free(UnixSocket* sock)
{
	free(sock->path);
	free(((Socket*)sock)->address);
	free(sock);
}
