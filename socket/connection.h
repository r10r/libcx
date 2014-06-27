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
typedef void* F_GetData (Connection* conn);
typedef void F_SetData (Connection* conn, void* data);
typedef int F_GetId (Connection* conn);

/* protocol callbacks ? */
struct cx_connection_callbacks_t
{
	F_ConnectionCallback* on_start;
	F_ConnectionCallback* on_close;         /* callback to free additional resource data here */
	F_ConnectionCallback* on_error;

	F_RequestCallback* on_request;
	F_ResponseCallback* on_response;
};

/* created by the connection watcher */
struct cx_connection_t
{
	int error;
	int error_errno;

	/* user API callbacks */
	ConnectionCallbacks* callbacks;

	/* list of pending responses */
	Queue* response_queue;

	/* worker API implementation */
	F_ReceiveData* f_receive;
	F_ResponseCallback* f_send;
	F_ConnectionCallback* f_close;
	F_ConnectionCallback* f_close_write;
	F_ConnectionCallback* f_close_read;

	/* worker implementation specific connection state */
	void* state;

	F_GetData* f_get_serverdata;
	F_GetId* f_get_id;

	F_GetData* f_get_userdata;
	F_SetData* f_set_userdata;
};

Connection*
Connection_new(ConnectionCallbacks* callbacks);

void
Connection_free(Connection* c);


#define CXDBG(conn, message) \
	XFDBG("Connection[%d] - " message, conn->f_get_id(conn))

#define CXFDBG(conn, format, ...) \
	XFDBG("Connection[%d] - " format, conn->f_get_id(conn), __VA_ARGS__)

#define CXERR(conn, message) \
	XFERR("Connection[%d] - " message, conn->f_get_id(conn))

#define CXFERR(conn, format, ...) \
	XFERR("Connection[%d] - " format, conn->f_get_id(conn), __VA_ARGS__)

#define CXWARN(conn, message) \
	XFWARN("Connection[%d] - " message, conn->f_get_id(conn))

#define CXFWARN(conn, format, ...) \
	XFWARN("Connection[%d] - " format, conn->f_get_id(conn), __VA_ARGS__)

#define CXERRNO(conn, message) \
	XFERRNO("Connection[%d] - " message, conn->f_get_id(conn))

#define CXFERRNO(conn, format, ...) \
	XFERRNO("Connection[%d] - " format, conn->f_get_id(conn), __VA_ARGS__)


#endif
