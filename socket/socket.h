#ifndef _CX_SOCKET_H
#define _CX_SOCKET_H

#include <sys/socket.h> /* guess what ;) */
#include "base/base.h"
#include "base/debug.h"

#define SOCK_BACKLOG 512

/*
 * gee all these function either define the error response
 * ore the success response
 */
typedef enum cx_socket_status_t
{
	SOCKET_INITIALIZED,
	SOCKET_CREATED,
	SOCKET_BIND,
	SOCKET_LISTEN,
	SOCKET_CONNECTED,
	SOCKET_ERROR_INVALID_ADDRESS,   /* Unparsable IPv4 address | Unix Socket path limit (108 tokens) exceeded */
	SOCKET_ERROR_ERRNO              /* errno holds error information */
} SocketStatus;

typedef struct cx_socket_t
{
	int namespace;
	int style;
	int protocol;
	const char* error_message; /* TODO define string errors (like strerror) */
	SocketStatus status;
	int fd;
	struct sockaddr* address;
	socklen_t address_size;
	int backlog;
} Socket;

/* [ server ] */

SocketStatus
Socket_serve(Socket* socket);

SocketStatus
Socket_bind(Socket* socket);

SocketStatus
Socket_listen(Socket* socket);


/* [ client ] */

SocketStatus
Socket_use(Socket* sock);

SocketStatus
Socket_connect(Socket* sock);


/* [ shared ] */

SocketStatus
Socket_create(Socket* socket);

void
Socket_print_status(Socket* socket);


/* helper functions */

#define Socket_enable_option(socket, option) \
	enable_so_opt(socket->fd, option)

void
enable_so_opt(int fd, int option);

#endif
