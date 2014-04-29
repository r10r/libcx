#include <stdio.h>
#include <stdlib.h>
#include <websock/websock.h>
#include "socket/client.h"
#include "string/string_buffer.h"

struct upstream_t
{
	const char* path;
	int fd;
};

static void
proxy_payload(libwebsock_client_state* state, libwebsock_message* msg)
{
	StringBuffer* receive_buffer = StringBuffer_new(1024);
	libwebsock_context* ctx = (libwebsock_context*)state->ctx;
	struct upstream_t* upstream = ctx->user_data;

	upstream->fd = client_connect(upstream->path);
	fprintf(stderr, "%s:%d\n", upstream->path, upstream->fd);
	send_data(upstream->fd, msg->payload);
	shutdown(upstream->fd, SHUT_WR);

	StringBuffer_fdload(receive_buffer, upstream->fd, 512);
	receive_response(upstream->fd);
	close(upstream->fd);

	/* send response back to ws client */
	libwebsock_send_text(state, StringBuffer_value(receive_buffer));
}

//basic onmessage callback, prints some information about this particular message
//then echos back to the client.
static int
onmessage(libwebsock_client_state* state, libwebsock_message* msg)
{
	fprintf(stderr, "Received message from client: %d\n", state->sockfd);
	fprintf(stderr, "Message opcode: %d\n", msg->opcode);
	fprintf(stderr, "Payload Length: %llu\n", msg->payload_len);
	fprintf(stderr, "Payload: %s\n", msg->payload);

	proxy_payload(state, msg);
	return 0;
}

static int
onopen(libwebsock_client_state* state)
{
	fprintf(stderr, "onopen: %d\n", state->sockfd);
	return 0;
}

static int
onclose(libwebsock_client_state* state)
{
	fprintf(stderr, "onclose: %d\n", state->sockfd);
	return 0;
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
		fprintf(stderr, "Error during libwebsock_init.\n");
		exit(1);
	}
	libwebsock_bind(ctx, "0.0.0.0", argv[1]);
	fprintf(stderr, "libwebsock listening on port %s\n", argv[1]);
	fprintf(stderr, "upstream to socket %s\n", argv[2]);

	struct upstream_t upstream = { .path = argv[2], .fd = -1 };

	ctx->user_data = &upstream;
	ctx->onmessage = onmessage;
	ctx->onopen = onopen;
	ctx->onclose = onclose;
	libwebsock_wait(ctx);
	//perform any cleanup here.
	fprintf(stderr, "Exiting.\n");
	return 0;
}
