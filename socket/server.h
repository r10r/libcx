#ifndef _CX_SERVER_H
#define _CX_SERVER_H

#include <stdio.h>      /* puts ... */
#include <unistd.h>     /* STDIN_FILENO */
#include <stdlib.h>     /* exit */
#include <string.h>     /* strlen */
#include <pthread.h>

#include "../base/base.h"
#include "../base/ev.h"
#include "../list/list.h"
#include "socket.h"

typedef struct cx_server_t Server;

typedef enum cx_server_event_t
{
	SERVER_START,
	SERVER_SHUTDOWN
} ServerEvent;

typedef void F_ServerHandler (Server* server, ServerEvent event);

struct cx_server_t
{
	int backlog; /* maximum pending connections */
	ev_loop* loop;
	ev_timer shutdown_watcher;
	ev_signal sigint_watcher;
	List* workers;
	Socket* socket;

	F_ServerHandler* f_server_handler;
};

Server*
Server_new(void);

void
Server_init(Server* server);

void
Server_free(Server* server);

int
Server_start(Server* server);

void
Server_shutdown(Server* server);

#endif
