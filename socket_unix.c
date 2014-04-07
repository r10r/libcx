#include "socket_unix.h"

UnixSocket*
UnixSocket_new(const char* path)
{
	UnixSocket* unix_socket = calloc(1, sizeof(UnixSocket));
	Socket* socket = (Socket*)unix_socket;

	socket->namespace = PF_LOCAL; /* == PF_UNIX */
	socket->style = SOCK_STREAM;
	socket->protocol = 0;

	unix_socket->path = strdup(path); // TODO free

	/* check path length */
	if (strlen(path) <= UNIX_PATH_MAX)
	{
		struct sockaddr_un* address = calloc(1, sizeof(struct sockaddr_un));
		socket->address = (struct sockaddr*)address;
		socket->address_size = sizeof(struct sockaddr_un);
		address->sun_family = PF_LOCAL;
		sprintf(address->sun_path, "%s", path);
		socket->status = SOCKET_INITIALIZED;
	}
	else
		socket->status = SOCKET_ERROR_INVALID_ADDRESS; // TODO set socket->error_message

	return unix_socket;
}

void
UnixSocket_free(UnixSocket* socket)
{
	free(socket->path);
	free(((Socket*)socket)->address);
	free(socket);
}
