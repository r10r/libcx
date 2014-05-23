#ifndef _CX_CONNECTION_H
#define _CX_CONNECTION_H

#include <stdlib.h>
#include <sys/socket.h>
#include <fcntl.h>      /* fcntl, to make socket non-blocking */

#include "base/ev.h"
#include "base/base.h"
#include "string/string_buffer.h"
#include "request.h"
#include "server.h" /* FIXME circular inclusion */
#include "worker.h"

// TODO move to socket.h
/* set given file descriptor as non-blocking */
#define unblock(fd) \
	fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK)

typedef struct cx_connection_t Connection;
typedef void F_ConnectionDataHandler (Connection* connection);
typedef void F_ConnectionDataCallback (ev_loop* loop, ev_io* w, int revents);

/* created by the connection watcher */
struct cx_connection_t
{
	/* FIXME limited to unix socket connections ? */
	int fd;

	/* watch for incomming data*/
	ev_io receive_data_watcher;
	ev_io send_data_watcher;

	// set the buffer to receive the data (function ?)
	F_ConnectionDataHandler* f_receive_data_handler;
	F_ConnectionDataHandler* f_send_data_handler;

	Worker* worker;

	void* data;
};

typedef struct cx_send_buffer_t SendBuffer;
typedef void F_SendFinished (Connection* conn, SendBuffer* unit);

struct cx_send_buffer_t
{
	StringBuffer* buffer;
	F_SendFinished* f_send_finished;
	size_t ntransmitted;
};

SendBuffer*
SendBuffer_new(StringBuffer* buffer, F_SendFinished* f_finished);

void
SendBuffer_free(SendBuffer* unit);

Connection*
Connection_new(Worker* worker, int fd);

void
Connection_init(Connection* connection, Worker* worker, int fd);

void
Connection_free(Connection* c);

void
Connection_start(Connection* c);

void
Connection_close_read(Connection* conn);

void
Connection_close_write(Connection* conn);

void
Connection_close(Connection* c);

void
Connection_send_buffer(Connection* c, StringBuffer* buf);

void
Connection_send_blocking(Connection* c, const char* data, size_t length);

#define Connection_start_write(conn) \
	ev_io_start(conn->worker->loop, &conn->send_data_watcher)

#define Connection_stop_write(conn) \
	ev_io_stop(conn->worker->loop, &conn->send_data_watcher)

#define Connection_start_read(conn) \
	ev_io_start(conn->worker->loop, &conn->receive_data_watcher);

#define Connection_stop_read(conn) \
	ev_io_stop(conn->worker->loop, &conn->receive_data_watcher);

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
