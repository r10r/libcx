#ifndef _CX_CONNECTION_H
#define _CX_CONNECTION_H

#include <stdlib.h>
#include <sys/socket.h>

#include "libcx-base/ev.h"
#include "libcx-base/base.h"
#include "libcx-string/string.h"
#include "request.h"

#include <fcntl.h>      /* fcntl, to make socket non-blocking */
/* set given file descriptor as non-blocking */
#define unblock(fd) \
	fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK)

typedef enum connection_event_t
{
	CONNECTION_EVENT_ACCEPTED,      /* new connection */
	CONNECTION_EVENT_DATA,
	CONNECTION_EVENT_CLOSE_READ,    /* client closed the writing end, there is no more data to read */
	CONNECTION_EVENT_RECEIVE_TIMEOUT,
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
	int connection_fd;

	ev_loop *loop;

	// the current outgoing response (shifted / cleared by send_data_watcher)
	StringBuffer *send_buffer;
//	List *send_buffer; /* FIFO (push | shift) */

	/* watch for incomming data*/
	ev_io receive_data_watcher;
	/* watch for socket becoming writable */
	ev_io send_data_watcher;

	// set the buffer to receive the data (function ?)
	F_ConnectionDataHandler *f_data_handler;
	F_ConnectionHandler *f_handler;

	void *data;
};


Connection*
Connection_new(ev_loop *loop, int fd);

void
Connection_start(Connection *c);

void
Connection_free(Connection *c);

void
Connection_close(Connection *c);

void
Connection_send(Connection *c, char *data, size_t length);

#endif
