#include "connection.h"

Connection*
Connection_new(Worker* worker, int fd)
{
	Connection* connection = cx_alloc(sizeof(Connection));

	Connection_init(connection, worker, fd);
	return connection;
}

void
Connection_init(Connection* connection, Worker* worker, int fd)
{
	connection->worker = worker;
	connection->fd = fd;
}

void
Connection_free(Connection* connection)
{
	cx_free(connection);
}

void
Connection_start(Connection* connection)
{
	unblock(connection->fd);
	ev_io_init(&connection->receive_data_watcher, receive_data_callback, connection->fd, EV_READ);
	ev_io_start(connection->worker->loop, &connection->receive_data_watcher);
}

void
Connection_close(Connection* connection)
{
	XFDBG("Closing connection[%d]", connection->fd);
	// shut down the connection
	ev_io_stop(connection->worker->loop, &connection->receive_data_watcher);
	close(connection->fd);
	Connection_free(connection);
}

/* sends data immediately */
void
Connection_send(Connection* c, const char* data, size_t length)
{
	/* FIXME this is inefficient brute force sending using busy waiting - use the event loop instead */
	size_t remaining = length;
	size_t processed = 0;
	ssize_t nsend = 0;

	while (remaining > 0)
	{
		nsend = send(c->fd, data + processed, remaining, 0);

		if (nsend == -1)
		{
			if (errno != EWOULDBLOCK || errno != EAGAIN)
				break;
		}
		else
		{
			XFDBG("Sent bytes (%zu of %zu)", nsend, length);
			remaining -= (size_t)nsend;
			processed += (size_t)nsend;
		}
	}

// @from write function documentation
//	This function returns the number of bytes transmitted, or -1 on failure. If the socket is nonblocking, then
//	 send (like write) can return after sending just part of the data. , for information about nonblocking mode.
//
//	Note, however, that a successful return value merely indicates that the message has been sent without
//	 error, not necessarily that it has been received without error.
	if (nsend == -1)
	{
		XFERRNO("Writing to connection %d", c->fd);
		c->f_handler(c, CONNECTION_EVENT_ERROR_WRITE);
	}

//	ev_io_init(&c->send_data_watcher, send_data_callback, c->connection_fd, EV_WRITE);
//	ev_io_start(c->loop, &c->send_data_watcher);
}

/*
 * @see man 1 read
 * A value of zero indicates end-of-file
 * (except if the value of the size argument is also zero).
 * This is not considered an error. If you keep calling read while at end-of-file,
 * it will keep returning zero and doing nothing else.
 */
void
receive_data_callback(ev_loop* loop, ev_io* w, int revents)
{
	Connection* connection = container_of(w, Connection, receive_data_watcher);

	ssize_t nread = connection->f_data_handler(connection);

	if (nread > 0)
	{
		XFLOG("Connection[%d] - received %zu bytes", connection->fd, nread);
		connection->f_handler(connection, CONNECTION_EVENT_DATA);
	}
	else if (nread == 0)
	{
		XFLOG("Connection[%d] - received EOF", connection->fd);
		ev_io_stop(loop, w); /* stop reading from socket */
		connection->f_handler(connection, CONNECTION_EVENT_CLOSE_READ);
	}
	else if (nread < 0)
	{
		/* errno is thread local see http://stackoverflow.com/questions/1694164/is-errno-thread-safe */
		XFLOG("Connection[%d] - received error %d: %s", connection->fd, errno, strerror(errno));
		connection->error = errno;
		connection->f_handler(connection, CONNECTION_EVENT_ERRNO);
	}
}
