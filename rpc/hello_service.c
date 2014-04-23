#include "hello_service.h"

static void
hello(const char* name)
{
	printf("Hello %s\n", name);
}

RPC(set_param, hello, 0, myparam, const char*, RPC_Request_get_param_string_value, RPC_String, 0)
RPC(param_list, hello)
{
	&RPC(param, hello, myparam)
};
RPC(method, hello)
{
	hello(RPC(get_param, hello, myparam));
}

RPC(set_param, foo, 0, bar, const char*, RPC_Request_get_param_string_value, RPC_String, 0)
RPC(param_list, foo)
{
	&RPC(param, foo, bar)
};
RPC(method, foo)
{
	hello(RPC(get_param, foo, bar));
}

RPC(set_param, blub, 0, blubber, const char*, RPC_Request_get_param_string_value, RPC_String, 0)
RPC(set_param, blub, 1, fooo, const char*, RPC_Request_get_param_string_value, RPC_String, 0)
RPC(param_list, blub)
{
	&RPC(param, blub, blubber),
	&RPC(param, blub, fooo)
};
RPC(method, blub)
{
	hello(RPC(get_param, blub, blubber));
	hello(RPC(get_param, blub, fooo));
}

RPC(method, lonely)
{
	hello("I've no params");
}

// TODO method without params ?

/* export RPC_Method definitions */
RPC(export, foo);
RPC(export, hello);
RPC(export, blub);

RPC(export_without_params, lonely);
