#ifndef _CX_HELLO_SERVICE_H
#define _CX_HELLO_SERVICE_H

#define RPC_NS EchoService_
#include "jsrpc.h"

/* export each function + method definition */
RPC_single_string_param(echo, 0, input, 0)
RPC_export(echo)

RPC_single_double_param(echo_double, 0, input, 0)
RPC_export(echo_double)

RPC_single_longlong_param(echo_longlong, 0, input, 0)
RPC_export(echo_longlong)

/* export all methods with a macro */
#define EchoService_methods \
	RPC_public_name(echo), \
	RPC_public_name(echo_double), \
	RPC_public_name(echo_longlong)

#endif
