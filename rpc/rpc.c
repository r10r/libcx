#include "rpc.h"

const RPC_Method RPC_Method_none =  {
	.name		= NULL,
	.method		= NULL,
	.signature	= NULL,
	.param_count	= 0
};

void
RPC_Method_log(RPC_Method* method)
{
	printf("* method[%s] %p params:#%d\n", method->name, method->method, method->param_count);
	int i = 0;
	for (i = 0; i < method->param_count; i++)
	{
		RPC_Param* param = method->signature[i];
		printf("  param[%d] name:%s type:%d, flags:%d\n",
		       param->pos, param->name, param->type, param->flags);
	}
}

RPC_Method*
RPC_Request_lookup_method(RPC_Request* request, RPC_Method methods[])
{
	int i;

	for (i = 0; methods[i].name != NULL; i++)
		if (strcmp(methods[i].name, request->method_name) == 0)
			return &methods[i];

	return NULL;
}
