#ifndef _CX_RPC_CONNECTION_H
#define _CX_RPC_CONNECTION_H

#include "socket/server.h"
#include "socket/connection.h"
#include "socket/request.h"
#include "socket/ws/frame.h"

#include "rpc.h"

#define RPC_SERVER_BUFFER_SIZE 2048

typedef struct cx_rpc_server_t
{
	Server server;
	RPC_Method* methods;
} RPC_Server;

void
rpc_request_callback(Connection* conn, Request* request);

#endif
