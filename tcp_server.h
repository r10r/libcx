#ifndef _CX_UNIX_SERVER_H
#define _CX_UNIX_SERVER_H

#include "server.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>

typedef struct tcp_server_t
{
	Server server;
	uint16_t port;
	const char *ip;
	int fd;
} TCPServer;


typedef struct tcp_worker_t
{
	Worker worker;
	int server_fd;
	ev_io connection_watcher;
	List *requests;                 /* pending requests */

} TCPWorker;

TCPServer*
TCPServer_new(const char *ip, uint16_t port);

/*
 * @return the file descriptor for the server socket
 * @see man unix 7
 * @see http://www.cas.mcmaster.ca/~qiao/courses/cs3mh3/tutorials/socket.html
 * @see http://stackoverflow.com/questions/3689925/struct-sockaddr-un-v-s-sockaddr-clinux
 */
int
tcp_socket_connect(const char* ip, uint16_t port);

static void
tcp_connection_watcher(ev_loop *loop, ev_io *w, int revents);

#endif
