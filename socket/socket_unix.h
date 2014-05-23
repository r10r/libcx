#ifndef _CX_SOCKET_UNIX_H
#define _CX_SOCKET_UNIX_H

#include <sys/un.h>

#include "socket.h"

#define UNIX_PATH_MAX 108

typedef struct cx_socket_unix_t
{
	Socket socket;
	char* path;
} UnixSocket;

UnixSocket*
UnixSocket_new(const char* path);

void
UnixSocket_free(UnixSocket* socket);

#endif
