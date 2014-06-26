#ifndef _CX_SOCKET_H
#define _CX_SOCKET_H

#include <sys/socket.h> /* guess what ;) */

#include "../base/base.h"

#define SOCK_BACKLOG 512

/* set given file descriptor as non-blocking */
#define unblock(fd) \
	fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK)

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
	SocketStatus status;
	int fd;
	struct sockaddr* address;
	socklen_t address_size;
	int backlog;
} Socket;

#define Socket_new \
	cx_alloc(sizeof(Socket))

#define Socket_free(sock) \
	cx_free(sock)

/* [ server ] */

SocketStatus
Socket_serve(Socket* socket);

SocketStatus
Socket_bind(Socket* socket);

SocketStatus
Socket_listen(Socket* socket);

Socket*
Socket_accept(Socket* sock);


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

void
Socket_set_timeout(Socket* sock, long millis, int optname, const char* name);

#define Socket_set_timeout_receive(sock, millis) \
	Socket_set_timeout(sock, millis, SO_RCVTIMEO, "SO_RCVTIMEO")

#define Socket_set_timeout_send(sock, millis) \
	Socket_set_timeout(sock, millis, SO_SNDTIMEO, "SO_SNDTIMEO")

#define Socket_set_timeouts(sock, millis_rcvtimeo, millis_sndtimeo) \
	Socket_set_timeout_receive(sock, millis_rcvtimeo); \
	Socket_set_timeout_send(sock, millis_sndtimeo)

void
Socket_ignore_sigpipe(Socket* sock);

#endif
