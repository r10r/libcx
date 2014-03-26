#include "unix_server.h"

static void
unix_connection_watcher(ev_loop *loop, ev_io *w, int revents);

static Server*
unix_server_handler(Server *server, ServerEvent event, void *data);

static Worker*
unix_worker_handler(Worker *worker, WorkerEvent event);

static Connection*
unix_connection_handler(Connection *connection, ConnectionEvent event);

static ssize_t
unix_connection_data_handler(Connection *connection);

UnixServer*
UnixServer_new(const char *sock_path)
{
	UnixServer *unix_server = malloc(sizeof(UnixServer));

	unix_server->socket_path = sock_path;

	Server *server = (Server*)unix_server;
	Server_new(server);

	server->f_server_handler = unix_server_handler;
	server->f_worker_handler = unix_worker_handler;
	server->f_connection_handler = unix_connection_handler;
	server->f_connection_data_handler = unix_connection_data_handler;

	return unix_server;
}

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

#if (!defined (__linux__) && defined(__unix__)) || (defined(__APPLE__) && defined(__MACH__))
	// http://stackoverflow.com/questions/108183/how-to-prevent-sigpipes-or-handle-them-properly
	// http://nadeausoftware.com/articles/2012/01/c_c_tip_how_use_compiler_predefined_macros_detect_operating_system
	// both server and client socket must be protected against SIGPIPE
	enable_so_opt(fd, SO_NOSIGPIPE); /* do not send SIGIPIPE on EPIPE */
#endif

	return fd;
}

static void
timer_cb(ev_loop *loop, ev_timer *timer, int revents)
{
	printf("Hello from the manager\n");
}

static Server*
unix_server_handler(Server *server, ServerEvent event, void *data)
{
	UnixServer *unix_server = (UnixServer*)server;

	switch (event)
	{
	case SERVER_START:
	{
		XDBG("server event start");
		// ignore SIGPIPE if on linux
		// TODO check if required because libev already blocks signals
		// TODO check if workers are protected from SIGPIPE signals
#if defined(__linux__)
		signal(SIGPIPE, SIG_IGN);
#endif
		// connect to socket
		unix_server->fd = unix_socket_connect(unix_server->socket_path);
		// TODO start worker supervisor
		ev_timer *timer = malloc(sizeof(ev_timer));
		ev_timer_init(timer, timer_cb, 0., 15.);
		ev_timer_again(EV_DEFAULT, timer);
		break;
	}
	case SERVER_STOP:
		XDBG("server event stop");
		break;
	case SERVER_START_WORKER:
	{
		XDBG("server event start worker");
		// pass server socket from server to worker
		UnixWorker *worker = (UnixWorker*)data;
		worker->server_fd = unix_server->fd;
	}
	break;
	}

	return server;
}

static Worker*
unix_worker_handler(Worker *worker, WorkerEvent event)
{
	UnixWorker *unix_worker = (UnixWorker*)worker;

	switch (event)
	{
	case WORKER_EVENT_NEW:
		// called in main process
		unix_worker = malloc(sizeof(UnixWorker));
		Worker_new((Worker*)unix_worker);
		unix_worker->requests = List_new();
		break;
	case WORKER_EVENT_START:
		XDBG("worker start");
		/* called from within the worker thread */
		/* start connection watcher */
		ev_io_init(&unix_worker->connection_watcher, unix_connection_watcher, unix_worker->server_fd, EV_READ);
		ev_io_start(worker->loop, &unix_worker->connection_watcher);
		break;
	case WORKER_EVENT_STOP:
		XDBG("worker event stop");
		/* start shutdown watcher */
		// worker should exit the event loop
		// when all connections are handled (emitting the WORKER_EVENT_RELEASE in the server thread)
		ev_io_stop(worker->loop, &unix_worker->connection_watcher);
		break;
	case WORKER_EVENT_RELEASE:
		// FIXME server must know when worker exits (e.g monitor status in the monitor thread )
		List_free(unix_worker->requests);
		break;
	}
	return (Worker*)unix_worker;
}

#define DATA_RECEIVE_MAX 1024

static ssize_t
unix_connection_data_handler(Connection *connection)
{
	Request *request = (Request*)connection->data;
	RagelParser *parser = (RagelParser*)request->userdata;

	return StringBuffer_fdcat(parser->buffer, connection->connection_fd, DATA_RECEIVE_MAX);
}

static void
unix_connection_watcher(ev_loop *loop, ev_io *w, int revents)
{
	UnixWorker *unix_worker = container_of(w, UnixWorker, connection_watcher);
	Worker *worker = (Worker*)unix_worker;

	int client_fd = accept(unix_worker->server_fd, NULL, NULL);

	if (client_fd == -1)
		XFLOG("Worker[%lu] - failed to accept\n", worker->id);
	else
	{

#if (!defined (__linux__) && defined(__unix__)) || (defined(__APPLE__) && defined(__MACH__))
		/* do not send SIGIPIPE on EPIPE */
		enable_so_opt(client_fd, SO_NOSIGPIPE);
#endif

		XFLOG("Worker[%lu] - accepted connection on fd:%d\n", worker->id, client_fd);

		Connection *connection = Connection_new(loop, client_fd);
		connection->f_handler = worker->f_connection_handler;
		// FIXME set connection data handler from worker / server
		connection->f_data_handler = worker->f_connection_data_handler;
		connection->f_handler(connection, CONNECTION_EVENT_ACCEPTED);
		Connection_start(connection);
	}
}

static Connection*
unix_connection_handler(Connection *connection, ConnectionEvent event)
{
	Request *request = NULL;
	RagelParser *parser = NULL;
	Message *message = NULL;

	if (connection->data)
	{
		request = (Request*)connection->data;
		parser = (RagelParser*)request->userdata;
		if (parser)
			message = (Message*)parser->userdata;
	}

	switch (event)
	{
	case CONNECTION_EVENT_ACCEPTED:
		XDBG("connection accepted");
		// FIXME generate unique request id
		// FIXME make initial message buffer read size configurable
		request = Request_new(666);
		request->userdata = MessageParser_new(DATA_RECEIVE_MAX);
		connection->data = request;
		break;
	case CONNECTION_EVENT_DATA:
	{
		RagelParser_parse(parser);
		break;
	}
	case CONNECTION_EVENT_CLOSE_READ:
	{
		XDBG("close read");
		RagelParser_finish(parser);
		RagelParser_free(parser);
		// FIXME make something useful with the message
		Message_free(message);
		break;
	}
	case CONNECTION_EVENT_RECEIVE_TIMEOUT:
		XDBG("connection timeout");
		break;
	case CONNECTION_EVENT_ERRNO:
		XDBG("connection error");
		break;
	case CONNECTION_EVENT_ERROR_WRITE:
		XDBG("connection error write");
		break;
	}
	return connection;
}


