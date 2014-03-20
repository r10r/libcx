#ifndef _SERVER_H
#define _SERVER_H

#include <stdio.h>      /* puts ... */
#include <unistd.h>     /* STDIN_FILENO */

#include <sys/socket.h> /* guess what ;) */
#include <sys/un.h>
#include <stdlib.h>     /* exit */
#include <string.h>     /* strlen */

#include <pthread.h>

#include "libcx-base/ev.h"
#include "libcx-base/debug.h"
#include "worker.h"

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
	SERVER_STOP,
	WORKER_START
} ServerEvent;


typedef struct server_t Server;
// FIXME data argument is only for passing the  worker
typedef void F_ServerHandler (Server *server, ServerEvent event, void *data);
typedef void F_RequestHandler (Request *request);


struct server_t
{
	int worker_count;
	int backlog; /* maximum pending connections */
	ev_loop *loop;
	List *workers;

	F_ServerHandler *f_server_handler;
	F_ConnectionHandler *f_connection_handler;
	F_WorkerHandler *f_worker_handler;
	F_RequestHandler *f_request_handler;
};

typedef struct unix_server_t
{
	Server server;
	char *socket_path;
	int fd;
} UnixServer;


Server*
Server_new(void);

void
Server_free(Server *server);

int
Server_start(Server *server);

void
Server_stop(Server *server);


/* helper functions */

int
unix_socket_connect(const char *sock_path);

void
enable_so_opt(int fd, int option);

#endif

