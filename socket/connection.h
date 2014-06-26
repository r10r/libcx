#ifndef _CX_CONNECTION_H
#define _CX_CONNECTION_H

#include <stdlib.h>
#include <sys/socket.h>
#include <fcntl.h>      /* fcntl, to make socket non-blocking */

#include "../base/ev.h"
#include "../base/base.h"
#include "../list/queue.h"
#include "../string/string_buffer.h"
#include "request.h"

// TODO move to socket.h
/* set given file descriptor as non-blocking */
#define unblock(fd) \
	fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK)

typedef struct cx_connection_t Connection;

/* TODO distinguish between read and write errros */
typedef enum
{
	CONNECTION_ERROR_ERRNO
} ConnectionError;

#include "response.h"

typedef struct cx_connection_callbacks_t ConnectionCallbacks;

typedef void F_ConnectionCallback (Connection* conn);
typedef void F_RequestCallback (Connection* conn, Request* req);
typedef void F_ResponseCallback (Connection* conn, Response* req);

/* protocol callbacks ? */
struct cx_connection_callbacks_t
{
	F_ConnectionCallback* on_start;
	F_ConnectionCallback* on_close;
	F_ConnectionCallback* on_error;

	F_RequestCallback* on_request;
	F_ResponseCallback* on_response;
};

/* created by the connection watcher */
struct cx_connection_t
{
	/* FIXME limited to unix socket connections ? */
	int fd;

	int error;
	int error_errno;

	// TODO add method to read data

	Queue* response_queue; /* list of send buffers */

	// set the buffer to receive the data (function ?)
	F_ConnectionCallback* f_receive_data_handler;
	F_ConnectionCallback* f_send_data_handler;
	F_ResponseCallback* send;

	ConnectionCallbacks* callbacks;

	void* protocol_data;
	void* service_data;
	void* io_data;

	/* watch for incomming data --> this is libev specific */
	ev_loop* loop;
	ev_io receive_data_watcher;
	ev_io send_data_watcher;
};

Connection*
Connection_new(int fd);

void
Connection_init(Connection* connection, int fd);

void
Connection_free(Connection* c);


#define CXDBG(con, message) \
	XFDBG("Connection[%d] - " message, con->fd)

#define CXFDBG(con, format, ...) \
	XFDBG("Connection[%d] - " format, con->fd, __VA_ARGS__)

#define CXERR(con, message) \
	XFERR("Connection[%d] - " message, con->fd)

#define CXFERR(con, format, ...) \
	XFERR("Connection[%d] - " format, con->fd, __VA_ARGS__)

#define CXERRNO(con, message) \
	XFERRNO("Connection[%d] - " message, con->fd)

#define CXFERRNO(con, format, ...) \
	XFERRNO("Connection[%d] - " format, con->fd, __VA_ARGS__)


#endif
