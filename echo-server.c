#include "server.h"

#define READ_EOF 0
#define IO_ERR -1               /* recv/send or read/write error */

static int server_fd;

static void
on_client_write(ev_loop *loop, ev_io *w, int revents)
{
	Worker *worker = (Worker*)ev_userdata(loop);

	XFLOG("Worker[%lu] fd:%d - sending response\n", worker->id, w->fd);
	ev_io_stop(loop, w);
	char data[] = "foo";
	size_t data_length = strlen(data);
	ssize_t bytes = send(w->fd, data, data_length, 0);
	if (bytes < 0 || (int)bytes < (int)data_length)
		XFLOG("Worker[%lu] fd:%d - failed to send response\n", worker->id, w->fd);
//	thread_wait(100 * 1000000); /* delay used for testing purposes */
	close(w->fd);
	free(w);
}

static void
on_client_read(ev_loop *loop, ev_io *w, int revents)
{
	Worker *worker = (Worker*)ev_userdata(loop);
	char buf[200];

	XFLOG("Worker[%lu] fd:%d - read connection (revents:%d)\n", worker->id, w->fd, revents);
	/*
	 * @see man 1 read
	 * A value of zero indicates end-of-file (except if the value of the size argument is also zero).
	 * This is not considered an error. If you keep calling read while at end-of-file,
	 * it will keep returning zero and doing nothing else.
	 */

	ssize_t num_read = read(w->fd, buf, 200);

	if (num_read == IO_ERR)
	{
		XFLOG("Worker[%lu] fd:%d - failed to receive data\n", worker->id, w->fd);
		ev_io_stop(loop, w);
	}
	else
	{
		if (num_read == READ_EOF)
		{
			XFLOG("Worker[%lu] fd:%d - EOF\n", worker->id, w->fd);
			ev_io *send_response_w = malloc(sizeof(ev_io));
			ev_io_init(send_response_w, on_client_write, w->fd, EV_WRITE);
			ev_io_start(loop, send_response_w);
			ev_io_stop(loop, w); /* stop reading from socket */
			free(w);
		}
		else
			XFLOG("Worker[%lu] fd:%d - received data (length %lu)\n", worker->id, w->fd, num_read);
	}
}

static void
listen_to_client(ev_loop *loop, int fd)
{
	unblock(fd);
	ev_io *client_data_w = malloc(sizeof(ev_io));
	ev_io_init(client_data_w, on_client_read, fd, EV_READ);
	ev_io_start(loop, client_data_w);
}

// all watcher callbacks have a similar signature
// this callback is called when data is readable on stdin
static void
on_connection(ev_loop *loop, ev_io *w, int revents)
{
	Worker *worker = ev_userdata(loop);

//	ev_io_stop(loop, connection_watcher);
	XFLOG("Worker[%lu] fd:%d - new connection (revents:%d)\n",
	      worker->id, w->fd, revents);

	int client_fd = accept(server_fd, NULL, NULL);
	if (client_fd == ACCEPT_ERROR)
	{
		XFLOG("Worker[%lu] fd:%d - failed to accept\n", worker->id, w->fd);
		return;
	}
	else
	{
		enable_so_opt(client_fd, SO_NOSIGPIPE); /* do not send SIGIPIPE on EPIPE */
		XFLOG("Worker[%lu] fd:%d - accepted connection\n", worker->id, w->fd);
		// TODO start data watcher here

		listen_to_client(loop, client_fd);
	}
}

/* connection timeout */
static void
timeout_cb(ev_loop *loop, ev_timer *w, int revents)
{
	printf("At your service!\n");
	// this causes the innermost ev_run to stop iterating
//	ev_break(loop, EVBREAK_ALL);
}

int
main(void)
{
	ev_io connection_watcher;
	ev_timer timeout_watcher;
	// use the default event loop unless you have special needs
	ev_loop *loop = EV_DEFAULT;

#if EV_COMPAT3
	puts("EV_COMPAT3 is set");
#endif

	server_fd = unix_socket_connect("/tmp/echo.sock");
	if (server_fd == SOCKET_CONNECT_FAILED)
	{
		fprintf(stderr, "Error on socket connection. Exit now\n");
		exit(1);
	}

	/* get notified on incomming connections */
	ev_io_init(&connection_watcher, on_connection, server_fd, EV_READ);
	ev_io_start(loop, &connection_watcher);

	/* simple timer, repeating every 5 seconds */
	ev_timer_init(&timeout_watcher, timeout_cb, 0., 5.);
	ev_timer_again(loop, &timeout_watcher);

	ev_run(loop, 0);

	return 0;
}
