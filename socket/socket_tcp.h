#ifndef _CX_SOCKET_TCP_H
#define _CX_SOCKET_TCP_H

/*
 * @see man unix 7
 * @see http://www.cas.mcmaster.ca/~qiao/courses/cs3mh3/tutorials/socket.html
 * @see http://stackoverflow.com/questions/3689925/struct-sockaddr-un-v-s-sockaddr-clinux
 */

#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "socket.h"

typedef struct cx_socket_tcp_t
{
	Socket socket;
	char* ip;
	uint16_t port;
} TCPSocket;

TCPSocket*
TCPSocket_new(const char* ip, uint16_t port);

void
TCPSocket_free(TCPSocket* socket);

#endif
