#include "client.h"
/*
 * http://www.thomasstover.com/uds.html
 */

void send_data(int fd, const char *data)
{
	XFDBG("Sending data: %s", data);
	send(fd, data, strlen(data), 0);
}

int
client_connect(const char *sock_path)
{
	int sock;
	struct sockaddr_un address;

	if ((sock = socket(PF_LOCAL, SOCK_STREAM, 0)) == -1)
	{
		perror("socket");
		exit(1);
	}

	XDBG("Trying to connect...");

	address.sun_family = PF_LOCAL;
	strcpy(address.sun_path, sock_path);
	if (connect(sock, (struct sockaddr *)&address, sizeof(address)) == -1)
	{
		perror("connect");
		exit(1);
	}

	XDBG("Connected.");
	return sock;
}

void
receive_response(int sock)
{
	char receive_buffer[100];
	ssize_t receive_count;

	receive_count = recv(sock, receive_buffer, 100, 0);
	XFDBG("Received %zu", receive_count);

	if (receive_count > 0)
	{
		receive_buffer[receive_count] = '\0';
		XFDBG("Response : %s", receive_buffer);
	}
	else
		XDBG("Do not received nothing");
}
