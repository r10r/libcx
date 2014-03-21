#include "connection.h"

#define DATA_RECEIVE_MAX 1024

/*
 * @see man 1 read
 * A value of zero indicates end-of-file
 * (except if the value of the size argument is also zero).
 * This is not considered an error. If you keep calling read while at end-of-file,
 * it will keep returning zero and doing nothing else.
 */
static void
receive_data_callback(ev_loop *loop, ev_io *w, int revents)
{
	Connection *connection = container_of(w, Connection, receive_data_watcher);

	XFLOG("Connection[%d] - revents:%d\n", connection->connection_fd, revents);
	ssize_t nread = connection->f_data_handler(connection);

	if (nread > 0)
		connection->f_handler(connection, CONNECTION_EVENT_DATA);
	else if (nread == 0)
	{
		ev_io_stop(loop, w); /* stop reading from socket */
		connection->f_handler(connection, CONNECTION_EVENT_CLOSE_READ);
	}
	else if (nread < 0)
	{
		ev_io_stop(loop, w);
		connection->f_handler(connection, CONNECTION_EVENT_ERRNO);
	}
}

static void
send_data_callback(ev_loop *loop, ev_io *w, int revents)
{
	Connection *connection = container_of(w, Connection, send_data_watcher);

	XFLOG("Connection[%d] - sending response\n", w->fd);

	// iterate over list and remove elements after iteration
	String s;
//	while ((s = (String) List_shift()) != NULL)
//	{
////		connection->f_event_handler(connection, CONNECTION_EVENT_WRITE);
//		unsigned int length = String_length(s);
//		ssize_t bytes = send(w->fd, s, length, 0);
//		XFLOG("Connection[%d] write:%zu\n", bytes);
	// FIXME handle write error
//		if (bytes != EWOULDBLOCK)
//		if (bytes < 0 || (unsigned int) bytes < length)
//			connection->f_event_handler(connection, CONNECTION_EVENT_RECEIVE_EOF);
//	}
}

Connection*
Connection_new(ev_loop *loop, int fd)
{
	unblock(fd);

	Connection *c = malloc(sizeof(Connection));
	c->loop = loop;
	c->data = NULL;
	c->f_data_handler = NULL;
	c->f_handler = NULL;

	c->connection_fd = fd;
	return c;
}

void
Connection_start(Connection *c)
{
	ev_io_init(&c->receive_data_watcher, receive_data_callback, c->connection_fd, EV_READ);
	ev_io_start(c->loop, &c->receive_data_watcher);
}

void
Connection_free(Connection *c)
{
	free(c);
}

// CLOSE reading end on EOF ? (to signal client that we do not accept any data ?)
void
Connection_close(Connection *c)
{
	// shut down the connection
	ev_io_stop(c->loop, &c->receive_data_watcher);
	ev_io_stop(c->loop, &c->send_data_watcher);
	close(c->connection_fd);
}

/* sends data immediately */
void
Connection_send(Connection *c, char *data, size_t length)
{
	ssize_t nsend = send(c->connection_fd, data, length, 0);

// @from write function documentation
//	This function returns the number of bytes transmitted, or -1 on failure. If the socket is nonblocking, then
//	 send (like write) can return after sending just part of the data. , for information about nonblocking mode.
//
//	Note, however, that a successful return value merely indicates that the message has been sent without
//	 error, not necessarily that it has been received without error.
	if (nsend == -1)
		c->f_handler(c, CONNECTION_EVENT_ERROR_WRITE);

//	ev_io_init(&c->send_data_watcher, send_data_callback, c->connection_fd, EV_WRITE);
//	ev_io_start(c->loop, &c->send_data_watcher);
}
