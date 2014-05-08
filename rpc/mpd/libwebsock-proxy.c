#include <stdio.h>
#include <stdlib.h>
#include <websock/websock.h>
#include "socket/client.h"
#include "string/string_buffer.h"
#include "socket/socket.h"
#include "signal.h"

struct upstream_t
{
	const char* path;
	int fd;
	StringBuffer* receive_buffer;
};

static int
socket_connect(const char* sock_path)
{
	int sock;
	struct sockaddr_un address;

	if ((sock = socket(PF_LOCAL, SOCK_STREAM, 0)) == -1)
		perror("socket");

	XDBG("Trying to connect...");

	address.sun_family = PF_LOCAL;
	strcpy(address.sun_path, sock_path);
	if (connect(sock, (struct sockaddr*)&address, sizeof(address)) == -1)
		perror("connect");

	XDBG("Connected.");
	return sock;
}

static struct upstream_t*
Upstream_new(const char* path)
{
	struct upstream_t* upstream = cx_alloc(sizeof(struct upstream_t));

	upstream->path = path;
	upstream->fd =  socket_connect(upstream->path);
	if (upstream->fd != -1)
	{
		enable_so_opt(upstream->fd, SO_NOSIGPIPE);  /* do not send SIGIPIPE on EPIPE */
		upstream->receive_buffer = StringBuffer_new(1024);
	}
	return upstream;
}

static void
Upstream_free(struct upstream_t* upstream)
{
	if (upstream)
	{
		if (upstream->fd != -1)
		{
			shutdown(upstream->fd, SHUT_RDWR);
			close(upstream->fd);
		}
		StringBuffer_free(upstream->receive_buffer);
		cx_free(upstream);
	}
}

static void
proxy_payload(struct upstream_t* upstream, libwebsock_client_state* state, libwebsock_message* msg)
{
	if (upstream)
	{
		XFDBG("upstream socket: %s:%d", upstream->path, upstream->fd);
		send_data(upstream->fd, msg->payload);
		shutdown(upstream->fd, SHUT_WR);

		StringBuffer_fdload(upstream->receive_buffer, upstream->fd, 512);

		/* send response back to ws client */
		libwebsock_send_text(state, StringBuffer_value(upstream->receive_buffer));
	}
	else
		XDBG("Connection was closed an upstream removed");
}

static int
onmessage(libwebsock_client_state* state, libwebsock_message* msg)
{
	// TODO pool upstream connections
	libwebsock_context* ctx = (libwebsock_context*)(state->ctx);
	struct upstream_t* upstream = Upstream_new((const char*)ctx->user_data);

	state->data =  upstream;

	XFDBG("Received message from client: %d", state->sockfd);
	XFDBG("Message opcode: %d", msg->opcode);
	XFDBG("Payload Length: %llu", msg->payload_len);
	XFDBG("Payload: %s", msg->payload);
	XFDBG("State %p", (void*)state);

	if (upstream->fd != -1)
		proxy_payload(upstream, state, msg);
	else
		libwebsock_send_text(state, "ERROR");

	Upstream_free(upstream);
	state->data = NULL;
	return 0;
}

static int
onopen(libwebsock_client_state* state)
{
	XFDBG("onopen: %d", state->sockfd);
	state->data = NULL;
	return 0;
}

static int
onclose(libwebsock_client_state* state)
{
	XFDBG("onclose: %d --> state->data %p", state->sockfd, state->data);
	if (state->data)
		Upstream_free((struct upstream_t*)state->data);

	return 0;
}

static void
sighandler(int signal)
{
	XFDBG("Received signal %d. Ignore", signal);
}

/*
 * TODO proper arguments parsing using getopt
 */
int
main(int argc, char* argv[])
{
	libwebsock_context* ctx = NULL;

	if (argc != 3)
	{
		fprintf(stderr, "Usage: %s <port to listen on> <upstream socket>\n", argv[0]);
		fprintf(stderr, "Note: You must be root to bind to port below 1024\n");
		exit(0);
	}

	ctx = libwebsock_init();
	if (ctx == NULL)
	{
		XERR("Error during libwebsock_init.");
		exit(1);
	}
	char* port = argv[1];
	char* upstream_path = argv[2];

	libwebsock_bind(ctx, "0.0.0.0", port);
	fprintf(stderr, "listening on port: %s\n", port);
	fprintf(stderr, "using upstream socket: %s\n", upstream_path);

	ctx->user_data = upstream_path;
	ctx->onmessage = onmessage;
	ctx->onopen = onopen;
	ctx->onclose = onclose;

	signal(SIGUSR2, sighandler);
	libwebsock_wait(ctx);
	//perform any cleanup here.
	fprintf(stderr, "Exiting.\n");
	return 0;
}
