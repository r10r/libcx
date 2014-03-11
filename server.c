#include "server.h"
#include "libcx-workqueue/pool.h"

/*
 * http://stackoverflow.com/questions/108183/how-to-prevent-sigpipes-or-handle-them-properly
 *
 * client might abort with SIGPIPE
 */

void
enable_so_opt(int fd, int option)
{
	int enable = 1;

	setsockopt(fd, SOL_SOCKET, option, (void*)&enable, sizeof(enable));
}

#define READ_EOF 0
#define IO_ERR -1               /* recv/send or read/write error */

static void
thread_wait(int nanos)
{
	int sleep_nanos = nanos;
	struct timespec sleep_time;

	sleep_time.tv_sec = 0;
	sleep_time.tv_nsec = sleep_nanos;
	nanosleep(&sleep_time, NULL);
}

void
on_client_write(ev_loop *loop, ev_io *w, int revents)
{
	Worker *worker = (Worker *) ev_userdata(loop);
	XFLOG("Worker[%d] fd:%d - sending response\n", worker->id, w->fd);
	ev_io_stop(loop, w);
	char *data = "foo";
	int data_length = strlen(data);
	int bytes = send(w->fd, data, data_length, 0);
	if (bytes < data_length)
		XFLOG("Worker[%d] fd:%d - failed to send response\n", worker->id, w->fd);
//	thread_wait(100 * 1000000); /* delay used for testing purposes */
	close(w->fd);
	free(w);
}

void
on_client_read(ev_loop *loop, ev_io *w, int revents)
{
	Worker *worker = (Worker *) ev_userdata(loop);
	char buf[200];

	XFLOG("Worker[%d] fd:%d - read connection (revents:%d)\n", worker->id, w->fd, revents);
	/*
	 * @see man 1 read
	 * A value of zero indicates end-of-file (except if the value of the size argument is also zero).
	 * This is not considered an error. If you keep calling read while at end-of-file,
	 * it will keep returning zero and doing nothing else.
	 */

	int num_read = read(w->fd, buf, 200);

	if (num_read == IO_ERR)
	{
		XFLOG("Worker[%d] fd:%d - failed to receive data\n", worker->id, w->fd);
		ev_io_stop(loop, w);
	}
	else
	{
		if (num_read == READ_EOF)
		{
			XFLOG("Worker[%d] fd:%d - EOF\n", worker->id, w->fd);
			ev_io *send_response_w = malloc(sizeof(ev_io));
			ev_io_init(send_response_w, on_client_write, w->fd, EV_WRITE);
			ev_io_start(loop, send_response_w);
			ev_io_stop(loop, w); /* stop reading from socket */
			free(w);
		}
		else
			XFLOG("Worker[%d] fd:%d - received data (length %d)\n", worker->id, w->fd, num_read);
	}
}

int server_fd;

void
listen_to_client(ev_loop *loop, int fd)
{
	unblock(fd);
	ev_io *client_data_w = malloc(sizeof(ev_io));
	ev_io_init(client_data_w, on_client_read, fd, EV_READ);
	ev_io_start(loop, client_data_w);
}

// all watcher callbacks have a similar signature
// this callback is called when data is readable on stdin
void
on_connection(ev_loop *loop, ev_io *w, int revents)
{
		Worker *worker = ev_userdata(loop);
//	ev_io_stop(loop, connection_watcher);
		XFLOG("Worker[%d] fd:%d - new connection (revents:%d)\n",
			worker->id, w->fd, revents);

	int client_fd = accept(server_fd, NULL, NULL);
	if (client_fd == ACCEPT_ERROR)
	{
		XFLOG("Worker[%d] fd:%d - failed to accept\n", worker->id, w->fd);
		return;
	}
	else
	{
		enable_so_opt(client_fd, SO_NOSIGPIPE); /* do not send SIGIPIPE on EPIPE */
		XFLOG("Worker[%d] fd:%d - accepted connection\n", worker->id, w->fd);
		// TODO start data watcher here

		listen_to_client(loop, client_fd);
	}
}

/* connection timeout */
void
timeout_cb(ev_loop *loop, ev_timer *w, int revents)
{
	printf("At your service!\n");
	// this causes the innermost ev_run to stop iterating
//	ev_break(loop, EVBREAK_ALL);
}

/*
 * @return the file descriptor for the server socket
 * @see man unix 7
 * @see http://www.cas.mcmaster.ca/~qiao/courses/cs3mh3/tutorials/socket.html
 * @see http://stackoverflow.com/questions/3689925/struct-sockaddr-un-v-s-sockaddr-clinux
 */
int
unix_socket_connect(const char *sock_path)
{
	struct sockaddr_un address;
	int fd = SOCKET_CONNECT_FAILED;
	int sock_path_len;
	int ret;
	int address_size;

	sock_path_len = strlen(sock_path);
	address_size = sizeof(address);
	XFLOG("Starting on socket : [%s]\n", sock_path);

	/* check preconditions */
	if (sock_path_len > UNIX_PATH_MAX)
	{
		fprintf(stderr,
			"Socket path to long (%d). Path length is limited to %d tokens.\n",
			sock_path_len, UNIX_PATH_MAX);
		return SOCKET_CONNECT_FAILED;
	}

	/* create socket */
	fd = socket(PF_UNIX, SOCK_STREAM, 0);
	if (fd == SOCKET_CREATE_ERROR)
	{
		XERR("Failed to create socket");
		return SOCKET_CONNECT_FAILED;
	}

	/* start with a clean address structure */
	memset(&address, 0, sizeof(struct sockaddr_un));
	address.sun_family = PF_UNIX;
	sprintf(address.sun_path, "%s", sock_path);

	/* bind */
	unlink(sock_path);
	ret = bind(fd, (struct sockaddr *)&address, address_size);
	if (ret != BIND_SUCCESS)
	{
		XERR("Failed to bind to socket");
		return SOCKET_CONNECT_FAILED;
	}

	/* listen */
	ret = listen(fd, SOCK_BACKLOG);
	if (ret != LISTEN_SUCCESS)
	{
		XERR("Failed to listen to socket");
		return SOCKET_CONNECT_FAILED;
	}
	// both server and client socket must be protected against SIGPIPE
	enable_so_opt(fd, SO_NOSIGPIPE); /* do not send SIGIPIPE on EPIPE */
	return fd;
}

