// a single header file is required
#define EV_COMPAT3 0    /* disable pre 4.0 compatibility (enables ev_loop typedef) */
#include <ev.h>
#include <stdio.h>      /* puts ... */
#include <unistd.h>     /* STDIN_FILENO */
#include <fcntl.h>      /* fcntl, to make socket non-blocking */
#include "libcx-base/debug.h"

#include <sys/socket.h> /* guess what ;) */
#include <sys/un.h>
#include <stdlib.h>     /* exit */
#include <string.h>     /* strlen */


#define UNIX_PATH_MAX 108
#define SOCK_BACKLOG 128
/*
 * gee all these function either define the error response
 * ore the success response
 */
#define BIND_SUCCESS 0          /* socket successfully bind */
#define LISTEN_SUCCESS 0        /* listening on socket */
#define ACCEPT_ERROR -1         /* failed to accept socket */
#define SOCKET_CREATE_ERROR -1  /* failed to create socket */

/* return this when the server socket setup failed */
#define SOCKET_CONNECT_FAILED -1

/* set given file descriptor as non-blocking */
#define unblock(fd) \
	fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK)

/* server socket file descriptor */
extern int server_fd;

// every watcher type has its own typedef'd struct  with the name ev_TYPE
extern ev_io connection_watcher;
extern ev_io data_watcher;
extern ev_timer timeout_watcher;

void
enable_so_opt(int fd, int option);

void
listen_to_client(ev_loop *loop, int fd);

void
on_client_write(ev_loop *loop, ev_io *watcher, int revents);

void
on_client_read(ev_loop *loop, ev_io *data_watcher, int revents);

void
on_connection(ev_loop *loop, ev_io *connection_watcher, int revents);

void
timeout_cb(ev_loop *loop, ev_timer *timer, int revents);

int
unix_socket_connect(const char *sock_path);
