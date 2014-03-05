#include "client.h"

int main(int argc, char** argv)
{
	int sock;

	assert(argc > 1);

	sock = client_connect("/tmp/echo.sock");
	int i;
	for (i = 1; i < argc; i++)
		send_data(sock, argv[i]);
	shutdown(sock, SHUT_WR); /* indicates EOF on server side */
	receive_response(sock);
	close(sock);

	return 0;
}
