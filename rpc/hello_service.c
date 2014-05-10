#include "hello_service.h"

static void
hello(const char* name)
{
	printf("Hello %s\n", name);
}

RPC(method, hello)
{
	hello(RPC(get_param, hello, myparam));
}

RPC(method, foo)
{
	hello(RPC(get_param, foo, bar));
}

RPC(method, blub)
{
	hello(RPC(get_param, blub, blubber));
	hello(RPC(get_param, blub, fooo));
}

RPC(method, lonely)
{
	hello("I've no params");
}
