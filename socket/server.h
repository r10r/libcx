#ifndef _CX_SERVER_H
#define _CX_SERVER_H

#include <stdio.h>      /* puts ... */
#include <unistd.h>     /* STDIN_FILENO */
#include <stdlib.h>     /* exit */
#include <string.h>     /* strlen */
#include <pthread.h>

#include <libcx/base/base.h>
#include <libcx/base/ev.h>
#include <libcx/list/list.h>

#include "socket.h"

typedef struct cx_server_t Server;
typedef struct cx_server_callbacks_t ServerCallbacks;
typedef void F_ServerCallback (Server* server);

/* worker requires server typedef */
#include "worker.h"

struct cx_server_callbacks_t
{
	F_ServerCallback* on_start;
	F_ServerCallback* on_stop;
};

#define Server_callback(server_, _cb_) \
	if ((server_)->callbacks && (server_)->callbacks->_cb_) (server_)->callbacks->_cb_(server_)

struct cx_server_t
{
	int backlog; /* maximum pending connections */
	ev_loop* loop;
	ev_timer shutdown_watcher;
	ev_signal sigint_watcher;
	List* workers;
	Socket* socket;

	ServerCallbacks* callbacks;

	F_ServerCallback* f_start;
	F_ServerCallback* f_stop;

	void* data;                                     /* available in the connection via conn->get_serverdata() */
	void* userdata;                                 /* available in the connection via conn->get_userdata() */
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
Server_stop(Server* server);

#endif
