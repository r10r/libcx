#ifndef _CONNECTION_WORKER_H
#define _CONNECTION_WORKER_H

#include "worker.h"
#include "connection.h"

#define DATA_RECEIVE_MAX 1024

typedef struct cx_connection_worker_t ConnectionWorker;
typedef Connection* F_CreateConnection (ConnectionCallbacks* callbacks);
typedef Connection* F_ManageConnection (ConnectionWorker* worker, Connection* connection);

void
connection_watcher(ev_loop* loop, ev_io* w, int revents);

struct cx_connection_worker_t
{
	Worker worker;
	int server_fd;
	ev_io connection_watcher;
	ConnectionCallbacks* callbacks;

	F_CreateConnection* f_connection_create;        /* factory method to create a new connection */

	F_ManageConnection* f_connection_start;
	F_ManageConnection* f_connection_close;
	F_ManageConnection* f_connection_send;
	F_ManageConnection* f_connection_close_read;
	F_ManageConnection* f_connection_close_write;
};

ConnectionWorker*
ConnectionWorker_new(F_CreateConnection* f_create_connection, ConnectionCallbacks* connection_callbacks);

void
ConnectionWorker_init(ConnectionWorker* worker);

void
ConnectionWorker_run(Worker* worker);

void
Connection_write_simple(Connection* conn);

#define Connection_callback(conn, _cb_) \
	if (conn->callbacks->_cb_) conn->callbacks->_cb_(conn)


void
ConnectionWorker_start_connection(ev_loop* loop, Connection* conn);

void
Connection_close_read(ev_loop* loop, Connection* conn);

void
Connection_close_write(ev_loop* loop, Connection* conn);

void
Connection_close(ev_loop* loop, Connection* conn);

void
Connection_send(Connection* conn, Response* response);

#endif
