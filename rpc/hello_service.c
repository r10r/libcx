#include "hello_service.h"

static void
hello(const char* name)
{
	printf("Hello %s\n", name);
}

RPC(params, hello)
{
	{ "name", get_param_value_string, 0 }
};
RPC(method, hello)
{
	hello((char*)request->params[0]);
}

RPC(params, foo)
{
	{ "bar", get_param_value_string, 0 }
};
RPC(method, foo)
{
	hello((char*)request->params[0]);
}

RPC(params, blub)
{
	{ "blubber", get_param_value_string, 66 },
	{ "fooo", get_param_value_string, 33 }
};
RPC(method, blub)
{
	hello((char*)request->params[0]);
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
