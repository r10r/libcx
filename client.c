#include "client.h"
/*
 * http://www.thomasstover.com/uds.html
 */

void send_data(int fd, const char *data)
{
	printf("Sending data: %s\n", data);
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

	printf("Trying to connect...\n");

	address.sun_family = PF_LOCAL;
	strcpy(address.sun_path, sock_path);
	if (connect(sock, (struct sockaddr *)&address, sizeof(address)) == -1)
	{
		perror("connect");
		exit(1);
	}

	printf("Connected.\n");
	return sock;
}

void
receive_response(int sock)
{
	char receive_buffer[100];
	int receive_count;

	receive_count = recv(sock, receive_buffer, 100, 0);
	printf("Received %d\n", receive_count);

	if (receive_count > 0)
	{
		receive_buffer[receive_count] = '\0';
		printf("Response : %s\n", receive_buffer);
	}
	else
		printf("Do not received nothing\n");
}
