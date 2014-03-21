#ifndef _UNIX_SERVER_H
#define _UNIX_SERVER_H

#include "server.h"

#include <sys/socket.h> /* guess what ;) */
#include <sys/un.h>

#define UNIX_PATH_MAX 108

typedef struct unix_server_t
{
	Server server;
	char *socket_path;
	int fd;
} UnixServer;


typedef struct unix_worker_t
{
	Worker worker;
	int server_fd;
	ev_io connection_watcher;
	List *requests;                 /* pending requests */

} UnixWorker;

UnixServer*
UnixServer_new(const char *sock_path);

/*
 * @return the file descriptor for the server socket
 * @see man unix 7
 * @see http://www.cas.mcmaster.ca/~qiao/courses/cs3mh3/tutorials/socket.html
 * @see http://stackoverflow.com/questions/3689925/struct-sockaddr-un-v-s-sockaddr-clinux
 */
int
unix_socket_connect(const char *sock_path);

static void
unix_connection_watcher(ev_loop *loop, ev_io *w, int revents);

#endif
