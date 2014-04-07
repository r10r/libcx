#ifndef _CX_SOCKET_UNIX_H
#define _CX_SOCKET_UNIX_H

#include "socket.h"
#include <sys/un.h>

#define UNIX_PATH_MAX 108

typedef struct socket_unix_t
{
	Socket socket;
	const char* path;
} UnixSocket;

UnixSocket*
UnixSocket_new(const char* path);

void
UnixSocket_free(UnixSocket* socket);

#endif
