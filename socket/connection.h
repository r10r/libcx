#ifndef _CX_CONNECTION_H
#define _CX_CONNECTION_H

#include <stdlib.h>
#include <sys/socket.h>
#include <fcntl.h>      /* fcntl, to make socket non-blocking */

#include <libcx/base/base.h>
#include <libcx/list/queue.h>
#include <libcx/string/string_buffer.h>

#include "request.h"

typedef struct cx_connection_t Connection;

#include "response.h"

#define CX_CONNECTION_ERROR_READ  0x1
#define CX_CONNECTION_ERROR_WRITE 0x2
#define CX_CONNECTION_ERROR_ERRNO 0x4

typedef struct cx_connection_callbacks_t ConnectionCallbacks;

typedef void F_ConnectionCallback (Connection* conn);
typedef void F_RequestCallback (Connection* conn, Request* request);
typedef void F_SendResponse (Connection* conn, Response* response, F_ResponseFinished* f_on_finished);
typedef void F_ReceiveData (Connection* conn, int fd);
typedef void* F_GetData (Connection* conn);
typedef void F_SetData (Connection* conn, void* data);
typedef int F_GetId (Connection* conn);
typedef void F_FreeState (void* state);
typedef void F_ConnectionTimerStart (Connection* conn, unsigned interval_millis);

/* protocol callbacks */
struct cx_connection_callbacks_t
{
	F_ConnectionCallback* on_connect;       /* called when connection is accepted */
	F_ConnectionCallback* on_close;         /* called before connection is closed (e.g to release resources) */
	F_ConnectionCallback* on_error;
	F_ConnectionCallback* on_timeout;
	F_ConnectionCallback* on_start;                 /* called when PROTOCOL (e.g websockets) connection is established */

	F_RequestCallback* on_request;
};

/* created by the connection watcher */
struct cx_connection_t
{
	int error_type;
	int error_code;

	/* user API callbacks */
	ConnectionCallbacks* callbacks;

	/* list of pending responses */
	Queue* response_queue;

	/* worker API implementation */
	F_ConnectionCallback* f_start;          /* called when TRANSPORT (e.g TCP) connection is established */
	F_ReceiveData* f_receive;
	F_SendResponse* f_send;
	F_ConnectionCallback* f_close;

	/* must be called from the same thread !!! */
	F_ConnectionCallback* f_receive_enable;
	F_ConnectionCallback* f_receive_disable;
	F_ConnectionCallback* f_receive_close;

	F_ConnectionCallback* f_send_enable;
	F_ConnectionCallback* f_send_disable;
	F_ConnectionCallback* f_send_close;

	F_ConnectionTimerStart* f_timer_start;
	F_ConnectionCallback* f_timer_stop;

	/* worker implementation specific connection state */
	void* state;
	void* data;
	void* userdata;

	F_FreeState* f_free_state;

	F_GetData* f_get_serverdata;
	F_GetId* f_get_id;
};

#define Connection_callback(conn_, _cb_) \
	if ((conn_)->callbacks->_cb_) (conn_)->callbacks->_cb_(conn_)

#define Connection_error(conn_, type_, code_, msg_) \
	CXFERR(conn_, "error %d : %s", code_, msg_); \
	(conn_)->error_type = type_; \
	(conn_)->error_code = code_; \
	Connection_callback(conn_, on_error)

#define Connection_errno_read(conn_) \
	CXERRNO(conn_, "read error"); \
	(conn_)->error_type = CX_CONNECTION_ERROR_READ | CX_CONNECTION_ERROR_ERRNO; \
	(conn_)->error_code = errno; \
	Connection_callback(conn_, on_error)

#define Connection_errno_write(conn_) \
	CXERRNO(conn_, "write error"); \
	(conn_)->error_type = CX_CONNECTION_ERROR_WRITE | CX_CONNECTION_ERROR_ERRNO; \
	(conn_)->error_code = errno; \
	Connection_callback(conn_, on_error)

#define Connection_is_write_error(conn_) \
	((conn_)->error_code & CX_CONNECTION_ERROR_WRITE)

#define Connection_is_read_error(conn_) \
	((conn_)->error_code & CX_CONNECTION_ERROR_READ)

Connection*
Connection_new(ConnectionCallbacks* callbacks);

void
Connection_free(Connection* c);

void*
Connection_get_data(Connection* conn);

void
Connection_set_data(Connection* conn, void* data);

void*
Connection_get_userdata(Connection* conn);

void
Connection_set_userdata(Connection* conn, void* userdata);

#define CXLOG(conn, message) \
	XFLOG("Connection[%d] - " message, (conn)->f_get_id(conn))

#define CXFLOG(conn, format, ...) \
	XFLOG("Connection[%d] - " format, (conn)->f_get_id(conn), __VA_ARGS__)

#define CXDBG(conn, message) \
	XFDBG("Connection[%d] - " message, (conn)->f_get_id(conn))

#define CXFDBG(conn, format, ...) \
	XFDBG("Connection[%d] - " format, (conn)->f_get_id(conn), __VA_ARGS__)

#define CXERR(conn, message) \
	XFERR("Connection[%d] - " message, (conn)->f_get_id(conn))

#define CXFERR(conn, format, ...) \
	XFERR("Connection[%d] - " format, (conn)->f_get_id(conn), __VA_ARGS__)

#define CXWARN(conn, message) \
	XFWARN("Connection[%d] - " message, (conn)->f_get_id(conn))

#define CXFWARN(conn, format, ...) \
	XFWARN("Connection[%d] - " format, (conn)->f_get_id(conn), __VA_ARGS__)

#define CXERRNO(conn, message) \
	XFERRNO("Connection[%d] - " message, (conn)->f_get_id(conn))

#define CXFERRNO(conn, format, ...) \
	XFERRNO("Connection[%d] - " format, (conn)->f_get_id(conn), __VA_ARGS__)

#endif
