#include "server.h"

static void
free_worker(void *data)
{
	Worker *worker = (Worker*)data;

	Worker_stop(worker); // FIXME must signal worker to close all connections
	Worker_free(worker);
}

// TODO default to processor count (varies from machine to machine)
#define PROCESSOR_COUNT 4

Server*
Server_new()
{
	// FIXME pass server reference to constructor to allow inheritance
	Server *s = malloc(sizeof(UnixServer));

	s->worker_count = PROCESSOR_COUNT;
	s->backlog = 0;                         // TODO set reasonable default (currently unused)
	s->loop = EV_DEFAULT;
	s->workers = List_new();
	s->workers->f_node_data_free = free_worker;

	return s;
}

int
Server_start(Server *server)
{
	server->f_server_handler(server, SERVER_START, NULL);

	// start workers
	int i;
	for (i = 0; i < server->worker_count; i++)
	{
		Worker *worker = Worker_new((unsigned long)i);
		worker->f_handler = server->f_worker_handler;
		worker->f_connection_handler = server->f_connection_handler;
		List_push(server->workers, worker);

		// callback allows custom initialization of worker
		server->f_server_handler(server, WORKER_START, worker);

		Worker_start(worker);
	}
	ev_run(server->loop, 0);
	// TODO error handling
	return 0;
}

void
Server_free(Server *server)
{
	List_free(server->workers);
	free(server);
}

void
Server_stop(Server *server)
{
	Server_free(server);
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
	size_t sock_path_len;
	int ret;
	socklen_t address_size;

	sock_path_len = strlen(sock_path);
	address_size = sizeof(address);
	XFLOG("Starting on socket : [%s]\n", sock_path);

	/* check preconditions */
	if (sock_path_len > UNIX_PATH_MAX)
	{
		fprintf(stderr,
			"Socket path to long (%ld). Path length is limited to %d tokens.\n",
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
	XFLOG("Connected to: fd:%d [%s]\n", fd, sock_path);

	// both server and client socket must be protected against SIGPIPE
	enable_so_opt(fd, SO_NOSIGPIPE); /* do not send SIGIPIPE on EPIPE */
	return fd;
}

void
enable_so_opt(int fd, int option)
{
	int enable = 1;

	setsockopt(fd, SOL_SOCKET, option, (void*)&enable, sizeof(enable));
}
