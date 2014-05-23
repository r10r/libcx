#ifndef _CX_SERVER_TCP_H
#define _CX_SERVER_TCP_H

#include "umtp/message_parser.h"
#include "server.h"
#include "socket_tcp.h"
#include "worker_unix.h"


Server*
TCPServer_new(const char* ip, uint16_t port);

void
TCPServer_init(Server* server, const char* ip, uint16_t port);

void
TCPServer_free(Server* server);

#endif
