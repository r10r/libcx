#include <libcx/socket/socket_unix.h>
#include <pthread.h>
#include <unistd.h> /* close */

static void*
AcceptWorker_run(void* data)
{
	Socket* socket = (Socket*)data;

	while (1)
	{
		int fd = accept(socket->fd, NULL, NULL);
		if (fd == -1)
		{
			XERRNO("server failed to accept connection");
		}
		else
		{
			XFDBG("New connection on fd:%d", fd);
			close(fd);
		}
	}
}

int
main(int argc, const char** argv)
{
//	TCPSocket* socket = TCPSocket_new("0.0.0.0", 6666);
	UnixSocket* socket = UnixSocket_new("/tmp/simple.sock");

	assert(argc == 2);

	int num_threads = atoi(argv[1]);
	pthread_t* threads = cx_alloc(sizeof(pthread_t) * (size_t)num_threads);

	if (Socket_serve((Socket*)socket) != SOCKET_LISTEN)
	{
		Socket_print_status((Socket*)socket);
		exit(1);
	}

	int i;
	for (i = 0; i < num_threads; i++)
	{
		pthread_create(threads + i, NULL, AcceptWorker_run, socket);
	}

	for (i = 0; i < num_threads; i++)
	{
		pthread_join(threads[i], NULL);
	}
}
