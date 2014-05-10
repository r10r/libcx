#ifndef _CX_HELLO_SERVICE_H
#define _CX_HELLO_SERVICE_H

#include "jsrpc.h"

/* namespace  declaration */
#undef RPC
#define RPC(action, ...) RPC_ ## action(HelloWorld, __VA_ARGS__)

/* export each function + method definition */
RPC(public, foo)
RPC(public, hello)
RPC(public, blub)
RPC(public, lonely)

/* export all methods with a macro */
#define HelloWorld_methods \
	RPC(public_name, foo), \
	RPC(public_name, hello), \
	RPC(public_name, blub), \
	RPC(public_name, lonely) \


#endif
