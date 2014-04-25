#ifndef _CX_HELLO_SERVICE_H
#define _CX_HELLO_SERVICE_H

#include "jsrpc.h"

/* namespace  declaration */
#undef RPC
#define RPC(action, ...) RPC_ ## action(HelloWorld, __VA_ARGS__)

RPC(single_string_param, hello, 0, myparam, 0)
RPC(export, hello)

RPC(single_string_param, foo, 0, bar, 0)
RPC(export, foo)

RPC(set_param_string, blub, 0, blubber, 0)
RPC(set_param_string, blub, 1, fooo, 0)
RPC(param_list, blub)
{
	&RPC(param, blub, blubber),
	&RPC(param, blub, fooo)
};
RPC(export, blub)

RPC(export_without_params, lonely)

/* export all methods with a macro */
#define HelloWorld_methods \
	RPC(public_name, foo), \
	RPC(public_name, hello), \
	RPC(public_name, blub), \
	RPC(public_name, lonely)


#endif
