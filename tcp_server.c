#include "tcp_server.h"

static void
tcp_connection_watcher(ev_loop *loop, ev_io *w, int revents);

static Server*
tcp_server_handler(Server *server, ServerEvent event, void *data);

static Worker*
tcp_worker_handler(Worker *worker, WorkerEvent event);

static Connection*
tcp_connection_handler(Connection *connection, ConnectionEvent event);

static ssize_t
tcp_connection_data_handler(Connection *connection);

TCPServer*
TCPServer_new(const char* ip, uint16_t port)
{
	TCPServer *tcp_server = malloc(sizeof(TCPServer));

	tcp_server->ip = ip;
	tcp_server->port = port;

	Server *server = (Server*)tcp_server;
	Server_new(server);

	server->f_server_handler = tcp_server_handler;
	server->f_worker_handler = tcp_worker_handler;
	server->f_connection_handler = tcp_connection_handler;
	server->f_connection_data_handler = tcp_connection_data_handler;

	return tcp_server;
}

int
tcp_socket_connect(const char *ip, uint16_t port)
{
	struct sockaddr_in address;
	int fd = SOCKET_CONNECT_FAILED;
	int ret;
	char ip_str[INET_ADDRSTRLEN];

	// convert ip address


	/* create socket */
	fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (fd == SOCKET_CREATE_ERROR)
	{
		XERR("Failed to create socket");
		return SOCKET_CONNECT_FAILED;
	}

	/* start with a clean address structure */
	memset(&address, 0, sizeof(struct sockaddr_in));
	address.sin_family = AF_INET;

	ret = inet_pton(AF_INET, ip, &(address.sin_addr));
	if (ret == 0)
		XFDBG("Unparsable IPv4 address : %s\n", ip);
	else if (ret == -1)
		XERR("Failed to convert IPv4 address");

	address.sin_port = htons(port);

	// ------ SNIP unix/tcp specifics are solved here

	/* bind */
	socklen_t address_size = sizeof(address);
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
	XFLOG("Connected to: fd:%d [%s:%d]\n", fd, ip, port);

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
tcp_server_handler(Server *server, ServerEvent event, void *data)
{
	TCPServer *tcp_server = (TCPServer*)server;

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
		tcp_server->fd = tcp_socket_connect(tcp_server->ip, tcp_server->port);
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
		TCPWorker *worker = (TCPWorker*)data;
		worker->server_fd = tcp_server->fd;
	}
	break;
	}

	return server;
}

static Worker*
tcp_worker_handler(Worker *worker, WorkerEvent event)
{
	TCPWorker *tcp_worker = (TCPWorker*)worker;

	switch (event)
	{
	case WORKER_EVENT_NEW:
		// called in main process
		tcp_worker = malloc(sizeof(TCPWorker));
		Worker_new((Worker*)tcp_worker);
		tcp_worker->requests = List_new();
		break;
	case WORKER_EVENT_START:
		XDBG("worker start");
		/* called from within the worker thread */
		/* start connection watcher */
		ev_io_init(&tcp_worker->connection_watcher, tcp_connection_watcher, tcp_worker->server_fd, EV_READ);
		ev_io_start(worker->loop, &tcp_worker->connection_watcher);
		break;
	case WORKER_EVENT_STOP:
		XDBG("worker event stop");
		/* start shutdown watcher */
		// worker should exit the event loop
		// when all connections are handled (emitting the WORKER_EVENT_RELEASE in the server thread)
		ev_io_stop(worker->loop, &tcp_worker->connection_watcher);
		break;
	case WORKER_EVENT_RELEASE:
		// FIXME server must know when worker exits (e.g monitor status in the monitor thread )
		List_free(tcp_worker->requests);
		break;
	}
	return (Worker*)tcp_worker;
}

#define DATA_RECEIVE_MAX 1024

static ssize_t
tcp_connection_data_handler(Connection *connection)
{
	Request *request = (Request*)connection->data;

	return Message_buffer_read(request->message, connection->connection_fd, DATA_RECEIVE_MAX);

}

static void
tcp_connection_watcher(ev_loop *loop, ev_io *w, int revents)
{
	TCPWorker *tcp_worker = container_of(w, TCPWorker, connection_watcher);
	Worker *worker = (Worker*)tcp_worker;

	// TODO print client information
	int client_fd = accept(tcp_worker->server_fd, NULL, NULL);

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
tcp_connection_handler(Connection *connection, ConnectionEvent event)
{
	Request *request;

	switch (event)
	{
	case CONNECTION_EVENT_ACCEPTED:
		XDBG("connection accepted");
		// FIXME generate unique request id
		// FIXME make initial message buffer read size configurable
		request = Request_new(666);
		request->message = Message_new(2048);
		connection->data = request;
		break;
	case CONNECTION_EVENT_DATA:
	{
		request = (Request*)connection->data;
		Message_parse(request->message);
		break;
	}
	case CONNECTION_EVENT_CLOSE_READ:
	{
		XDBG("close read");
		request = (Request*)connection->data;
		Message_parse_finish(request->message);
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


