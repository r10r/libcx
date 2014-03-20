#ifndef _CONNECTION_H
#define _CONNECTION_H

#include <stdlib.h>
#include <sys/socket.h>

#include "libcx-base/ev.h"

typedef enum connection_event_t
{
	CONNECTION_EVENT_NEW,           /* new connection */
	CONNECTION_EVENT_RECEIVE_DATA,
	CONNECTION_EVENT_CLOSE_READ,    /* client closed the writing end, there is no more data to read */
	CONNECTION_EVENT_NEW_MESSAGE,   /* a new message begins (for pipelining) */
//	CONNECTION_EVENT_CLOSE_WRITE, /* server closed writing end */
	CONNECTION_EVENT_END,           /* flushes send_buffer and close connection */
	CONNECTION_EVENT_RECEIVE_TIMEOUT,
	CONNECTION_EVENT_ERRNO,
	CONNECTION_EVENT_ERROR_WRITE
} ConnectionEvent;

typedef struct connection_t Connection;
typedef void F_ConnectionEventHandler (Connection *connection, ConnectionEvent event, void *data);


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
	ev_io *receive_data_watcher;
	/* watch for socket becoming writable */
	ev_io *send_data_watcher;

	// set the buffer to receive the data (function ?)

	// callback to
	F_ConnectionEventHandler *f_event_handler;

	/* something that has a buffer (that can be casted to a string buffer ?) */
	Request *request;

};


Connection*
Connection_new(ev_loop loop, int fd, int buffer_length);

void
Connection_free(Connection *c);

void
Connection_close(Connection *c);

void
Connection_send(Connection *c, String s);

#endif
