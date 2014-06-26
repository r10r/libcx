#ifndef _CX_CONNECTION_H
#define _CX_CONNECTION_H

#include <stdlib.h>
#include <sys/socket.h>
#include <fcntl.h>      /* fcntl, to make socket non-blocking */

#include "../base/base.h"
#include "../list/queue.h"
#include "../string/string_buffer.h"
#include "request.h"

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
typedef void F_ReceiveData (Connection* conn, int fd);

/* protocol callbacks ? */
struct cx_connection_callbacks_t
{
	F_ConnectionCallback* on_start;
	F_ConnectionCallback* on_close;         /* free additional resource data here */
	F_ConnectionCallback* on_error;

	F_RequestCallback* on_request;
	F_ResponseCallback* on_response;
};

/* created by the connection watcher */
struct cx_connection_t
{
	int error;
	int error_errno;

	// TODO add method to read data

	Queue* response_queue; /* list of send buffers */

	// set the buffer to receive the data (function ?)
	F_ReceiveData* f_receive;
	F_ResponseCallback* f_send;
	F_ConnectionCallback* f_close;
	F_ConnectionCallback* f_close_write;
	F_ConnectionCallback* f_close_read;

	ConnectionCallbacks* callbacks;

	void* worker_data;
	void* protocol_data;
};

Connection*
Connection_new(ConnectionCallbacks* callbacks);

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
