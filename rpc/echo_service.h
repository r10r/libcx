#ifndef _CX_HELLO_SERVICE_H
#define _CX_HELLO_SERVICE_H

#include "jsrpc.h"

/* namespace  declaration */
#undef RPC
#define RPC(action, ...) RPC_ ## action(Echo, __VA_ARGS__)

/* export each function + method definition */
RPC(public, echo)

/* export all methods with a macro */
#define Echo_methods \
	RPC(public_name, echo)

#endif
