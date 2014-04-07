#include "socket_tcp.h"

TCPSocket*
TCPSocket_new(const char* ip, uint16_t port)
{
	TCPSocket* tcp_socket = calloc(1, sizeof(TCPSocket));
	Socket* socket = (Socket*)tcp_socket;

	socket->namespace = PF_INET;
	socket->style = SOCK_STREAM;
	socket->protocol = IPPROTO_TCP;

	tcp_socket->ip = strdup(ip);

	struct sockaddr_in* address = calloc(1, sizeof(struct sockaddr_in));
	socket->address = (struct sockaddr*)address;
	socket->address_size = sizeof(struct sockaddr_in);
	address->sin_family = AF_INET;

	int ret = inet_pton(AF_INET, ip, &(address->sin_addr));
	if (ret == 1)
	{
		address->sin_port = htons(port);
		socket->status = SOCKET_INITIALIZED;
	}
	else if (ret == 0)
		socket->status = SOCKET_ERROR_INVALID_ADDRESS;
	else if (ret == -1)
		socket->status = SOCKET_ERROR_ERRNO;

	return tcp_socket;
}

void
TCPSocket_free(TCPSocket* socket)
{
	free(((Socket*)socket)->address);
	free(socket->ip);
	free(socket);
}
