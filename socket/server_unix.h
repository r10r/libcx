#ifndef _CX_SERVER_UNIX_H
#define _CX_SERVER_UNIX_H

#include "../umtp/message_parser.h"
#include "server.h"
#include "socket_unix.h"
#include "worker_unix.h"

Server*
UnixServer_new(const char* sock_path);

void
UnixServer_init(Server* server, const char* sock_path);

void
UnixServer_free(Server* server);

#endif
