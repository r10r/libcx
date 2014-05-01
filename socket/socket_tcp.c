#include "socket_tcp.h"

TCPSocket*
TCPSocket_new(const char* ip, uint16_t port)
{
	TCPSocket* tcp_sock = calloc(1, sizeof(TCPSocket));
	Socket* sock = (Socket*)tcp_sock;

	sock->namespace = PF_INET;
	sock->style = SOCK_STREAM;
	sock->protocol = IPPROTO_TCP;
	sock->backlog = SOCK_BACKLOG;

	tcp_sock->port = port;
	tcp_sock->ip = strdup(ip);

	struct sockaddr_in* address = calloc(1, sizeof(struct sockaddr_in));
	sock->address = (struct sockaddr*)address;
	sock->address_size = sizeof(struct sockaddr_in);
	address->sin_family = AF_INET;

	int ret = inet_pton(AF_INET, ip, &(address->sin_addr));
	if (ret == 1)
	{
		address->sin_port = htons(port);
		sock->status = SOCKET_INITIALIZED;
	}
	else if (ret == 0)
		sock->status = SOCKET_ERROR_INVALID_ADDRESS;
	else if (ret == -1)
		sock->status = SOCKET_ERROR_ERRNO;

	return tcp_sock;
}

void
TCPSocket_free(TCPSocket* sock)
{
	free(((Socket*)sock)->address);
	free(sock->ip);
	free(sock);
}
