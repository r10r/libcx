#include "client.h"
#include <pthread.h>
#include "base/debug.h"
#include "base/profile.h"
#include <assert.h>
#include "string/string_buffer.h"

typedef void F_Connected (TCPSocket* sock);

static int connections;
static const char* ip;
static uint16_t port;

static void
on_connect(TCPSocket* tcp)
{
	XFDBG("Connected with fd %d", tcp->socket.fd);
	StringBuffer* out = StringBuffer_from_printf(100, "foobar %d", rand());
	StringBuffer_write(out, tcp->socket.fd);

	shutdown(tcp->socket.fd, SHUT_WR); /* indicates EOF on server side */

	StringBuffer* in = StringBuffer_new(1024);
	StringBuffer_fdxload(in, tcp->socket.fd, 128, 0);

	assert(StringBuffer_equals(out, in));

	StringBuffer_free(out);
	StringBuffer_free(in);
}

static void*
thread_send_data(void* data)
{
	F_Connected* callback = on_connect;

	int i;

	PROFILE_BEGIN_FMT("Thread[%p] - Starting %d connections\n", (void*)(pthread_self()), connections);
	for (i = 0; i < connections; i++)
	{
		TCPSocket* tcp = TCPSocket_new(ip, port);

		XFDBG("Connection TCP socket: %s:%d", tcp->ip, tcp->port);

		SocketStatus status = Socket_use((Socket*)tcp);
		if (status != SOCKET_CONNECTED)
			perror("socket not connected:");

		assert(status == SOCKET_CONNECTED);

		callback(tcp);

		close(tcp->socket.fd);
		TCPSocket_free(tcp);
	}

	PROFILE_END
	return NULL;
}

int
main(int argc, char** argv)
{
	XASSERT(argc == 5,
		"usage: $0 THREADS CONNECTIONS IP PORT");

	int thread_count = atoi(argv[1]);
	connections = atoi(argv[2]);
	ip = argv[3];
	port = (uint16_t)atoi(argv[4]);

	pthread_t* threads = cx_alloc((size_t)thread_count * sizeof(pthread_t*));

	PROFILE_BEGIN_FMT("threads:%d requests/thread:%d\n", thread_count, connections);

	int i;
	for (i = 0; i < thread_count; i++)
		pthread_create(&threads[i], NULL, thread_send_data, NULL);


	for (i = 0; i < thread_count; i++)
		pthread_join(threads[i], NULL);


	PROFILE_END;

	cx_free(threads);
	return 0;
}
