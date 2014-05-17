#ifndef _CX_HELLO_SERVICE_H
#define _CX_HELLO_SERVICE_H

#define RPC_NS EchoService_
#include "jsrpc.h"

RPC_export(echo);
RPC_export(echo_double);
RPC_export(echo_longlong);

/* export all methods with a macro */
#define EchoService_methods \
	RPC_public_name(echo), \
	RPC_public_name(echo_double), \
	RPC_public_name(echo_longlong)

#endif
