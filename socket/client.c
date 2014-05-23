#include "client.h"

/*
 * http://www.thomasstover.com/uds.html
 */

void
send_data(int fd, const char* data)
{
	XFDBG("Sending data: %s", data);
	send(fd, data, strlen(data), 0);
}

int
client_connect(const char* sock_path)
{
	int sock;
	struct sockaddr_un address;

	if ((sock = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
	{
		perror("socket");
		exit(1);
	}

	XDBG("Trying to connect...");

	address.sun_family = AF_UNIX;
	strcpy(address.sun_path, sock_path);
	if (connect(sock, (struct sockaddr*)&address, sizeof(address)) == -1)
	{
		perror("connect");
		exit(1);
	}

	XDBG("Connected.");
	return sock;
}
