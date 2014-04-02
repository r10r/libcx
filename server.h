#ifndef _CX_SERVER_H
#define _CX_SERVER_H

#include <stdio.h>      /* puts ... */
#include <unistd.h>     /* STDIN_FILENO */

#include <stdlib.h>     /* exit */
#include <string.h>     /* strlen */

#include <pthread.h>

#include "libcx-base/base.h"
#include "libcx-base/ev.h"
#include "libcx-base/debug.h"
#include "libcx-list/list.h"

typedef struct server_t Server;

typedef enum server_event_t
{
	SERVER_START,
	SERVER_SHUTDOWN
} ServerEvent;

typedef void F_ServerHandler (Server *server, ServerEvent event);

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

struct server_t
{
	int backlog; /* maximum pending connections */
	ev_loop *loop;
	ev_timer shutdown_watcher;
	ev_signal sigint_watcher;
	List *workers;

	F_ServerHandler *f_server_handler;
};

Server*
Server_new(void);

void
Server_init(Server *server);

void
Server_free(Server *server);

int
Server_start(Server *server);

void
Server_shutdown(Server *server);


/* helper functions */

void
enable_so_opt(int fd, int option);

#endif

