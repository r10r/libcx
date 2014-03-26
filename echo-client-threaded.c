#include "client.h"
#include <pthread.h>
#include "libcx-base/debug.h"
#include "libcx-base/profile.h"

struct client_data_t
{
	const char *socket_path;
	int connections;
	const char *data;
};

static void*
thread_send_data(void *data)
{
	struct client_data_t *d = (struct client_data_t*)data;

	int i;

	for (i = 0; i < d->connections; i++)
	{
		int sock = client_connect(d->socket_path);
		send_data(sock, d->data);
		shutdown(sock, SHUT_WR); /* indicates EOF on server side */
		receive_response(sock);
		close(sock);
	}
	return NULL;
}

int main(int argc, char** argv)
{
	XASSERT(argc == 4,
		"usage: $0 THREADS CONNECTIONS DATA");

	int thread_count = atoi(argv[1]);
	int connections = atoi(argv[2]);
	char *data = argv[3];

	pthread_t *threads = malloc((size_t)thread_count * sizeof(pthread_t *));

	struct client_data_t d =
	{ .socket_path			= "/tmp/echo.sock",
	  .connections			= connections,
	  .data				= data };

	PROFILE_BEGIN_FMT("threads:%d requests/thread:%d\n", thread_count, connections);

	int i;
	for (i = 0; i < thread_count; i++)
	{
		pthread_create(&threads[i], NULL, thread_send_data, &d);
		pthread_join(threads[i], NULL);
	}

	PROFILE_END;

	free(threads);
	return 0;
}
