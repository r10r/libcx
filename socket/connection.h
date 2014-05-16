#ifndef _CX_CONNECTION_H
#define _CX_CONNECTION_H

#include <stdlib.h>
#include <sys/socket.h>

#include "base/ev.h"
#include "base/base.h"
#include "string/string_buffer.h"
#include "request.h"
#include "server.h" /* FIXME circular inclusion */
#include "worker.h"

#include <fcntl.h>      /* fcntl, to make socket non-blocking */

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

#define CXDBG(con, message) \
	XFDBG("Connection[%d] - " message, con->fd)

#define CXFDBG(con, message, ...) \
	XFDBG("Connection[%d] - " message, con->fd, __VA_ARGS__)

#define CXERRNO(con, message) \
	XFERRNO("Connection[%d] - " message, con->fd)

#define CXFERRNO(con, message, ...) \
	XFERRNO("Connection[%d] - " message, con->fd, __VA_ARGS__)


#endif
