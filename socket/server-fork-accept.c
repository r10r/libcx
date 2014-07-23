#include <libcx/socket/socket_tcp.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <sys/socket.h>


static void
run(TCPSocket* socket) __attribute__((noreturn));

static void
run(TCPSocket* socket)
{
	XFLOG("Child[%d] - listening to socket: %s:%d", getpid(), socket->ip, socket->port);
	while (1)
	{
		int fd = accept(socket->socket.fd, NULL, NULL);
		if (fd == -1)
		{
			XERRNO("server failed to accept connection");
		}
		else
		{
			XFDBG("Child[%d] - new connection on fd:%d", getpid(), fd);
			close(fd);
		}
	}
}

int
main(int argc, const char** argv)
{
	TCPSocket* socket = TCPSocket_new("0.0.0.0", 6666);

	assert(argc == 2);

	int num_procs = atoi(argv[1]);

	if (Socket_serve((Socket*)socket) != SOCKET_LISTEN)
	{
		Socket_print_status((Socket*)socket);
		exit(1);
	}

	XFLOG("Listening to socket: %s:%d", socket->ip, socket->port);

	int i;
	for (i = 0; i < num_procs; i++)
	{
		pid_t pid = fork();
		if (pid == 0)
		{
			// parent
			wait(NULL);
		}
		else
		{
			// child
			run(socket);
		}
	}
}
