#ifndef _CX_CONNECTION_H
#define _CX_CONNECTION_H

#include <stdlib.h>
#include <sys/socket.h>

#include "libcx-base/ev.h"
#include "libcx-base/base.h"
#include "libcx-string/string.h"
#include "request.h"
#include "server.h" /* FIXME circular inclusion */
#include "worker.h"

#include <fcntl.h>      /* fcntl, to make socket non-blocking */

// TODO move to socket.h
/* set given file descriptor as non-blocking */
#define unblock(fd) \
	fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK)

typedef enum connection_event_t
{
	CONNECTION_EVENT_DATA,
	/*
	 * client closed the writing end, there is no more data to read
	 * TODO CLOSE reading end on EOF ? (to signal client that we do not accept any data ?)
	 */
	CONNECTION_EVENT_CLOSE_READ,
	CONNECTION_EVENT_ERRNO,
	CONNECTION_EVENT_ERROR_WRITE
} ConnectionEvent;

typedef struct connection_t Connection;
typedef Connection* F_ConnectionHandler (Connection *connection, ConnectionEvent event);
typedef ssize_t F_ConnectionDataHandler (Connection *connection);

/* created by the connection watcher */
struct connection_t
{
	/* FIXME limited to unix socket connections ? */
	int fd;

	/* watch for incomming data*/
	ev_io receive_data_watcher;

	// set the buffer to receive the data (function ?)
	F_ConnectionDataHandler *f_data_handler;
	F_ConnectionHandler *f_handler;

	Worker *worker;

	void *data;
};

void
receive_data_callback(ev_loop *loop, ev_io *w, int revents);

Connection*
Connection_new(Worker *worker, int fd);

void
Connection_init(Connection *connection, Worker* worker, int fd);

void
Connection_free(Connection *c);

void
Connection_start(Connection *c);

void
Connection_close(Connection *c);

void
Connection_send(Connection *c, char *data, size_t length);

#endif
