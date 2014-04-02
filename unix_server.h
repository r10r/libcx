#ifndef _CX_UNIX_SERVER_H
#define _CX_UNIX_SERVER_H

#include "server.h"
#include "libcx-umtp/message_parser.h"

#include <sys/socket.h> /* guess what ;) */
#include <sys/un.h>

#include "unix_worker.h"

#define UNIX_PATH_MAX 108

typedef struct unix_server_t
{
	Server server;
	char *socket_path;
	int fd;
} UnixServer;

UnixServer*
UnixServer_new(const char *sock_path);

void
UnixServer_init(UnixServer *unix_server, const char *sock_path);

void
UnixServer_free(UnixServer *server);

/*
 * @return the file descriptor for the server socket
 * @see man unix 7
 * @see http://www.cas.mcmaster.ca/~qiao/courses/cs3mh3/tutorials/socket.html
 * @see http://stackoverflow.com/questions/3689925/struct-sockaddr-un-v-s-sockaddr-clinux
 */
int
unix_socket_connect(const char *sock_path);

void
unix_connection_watcher(ev_loop *loop, ev_io *w, int revents);

#endif
