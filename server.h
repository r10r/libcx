#ifndef _SERVER_H
#define _SERVER_H

#include <stdio.h>      /* puts ... */
#include <unistd.h>     /* STDIN_FILENO */
#include <fcntl.h>      /* fcntl, to make socket non-blocking */

#include <sys/socket.h> /* guess what ;) */
#include <sys/un.h>
#include <stdlib.h>     /* exit */
#include <string.h>     /* strlen */

#include <pthread.h>

#include "libcx-base/ev.h"
#include "libcx-base/base.h" /* container_of */
#include "libcx-base/debug.h"
#include "worker.h"
#include "request.h"
#include "connection.h"


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

// TODO add a separate logging thread
// * add shutdown handler and an error handler
// * add a worker monitor

typedef enum server_event_t
{
	SERVER_START,
	SERVER_STOP
} ServerEvent;


typedef struct server_t Server;
typedef void F_ServerEventHandler (Server *server, ServerEvent event);
typedef void F_RequestEventHandler (Request *request);


/* set given file descriptor as non-blocking */
#define unblock(fd) \
	fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK)

/* notify workers using ev_async (shutdown, restart, ...) */
struct server_t
{
	char *socket_path;
	int fd;
	int worker_count;
	int backlog; /* maximum pending connections */
	ev_loop *loop;
//	ev_timer *worker_health_watcher;        /* monitor workers health using ev async, move to worker pool ? */
	/* watcher to manage the worker pool ? */
	List *workers;

	F_ServerEventHandler *f_server_handler;
	F_ConnectionEventHandler *f_connection_handler;
	F_WorkerEventHandler *f_worker_handler;
	F_RequestEventHandler *f_request_handler;

};

/* helper functions */

int
unix_socket_connect(const char *sock_path);

void
enable_so_opt(int fd, int option);

Server*
Server_new(char *socket_path);

void
Server_free(Server *server);

int
Server_start(Server *server);

void
Server_stop(Server *server);


#endif

