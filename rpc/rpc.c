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
	printf("* method[%s] params:#%d\n", method->name, method->param_count);
	int i = 0;
	for (i = 0; i < method->param_count; i++)
	{
		RPC_Param* param = method->signature[i];
		printf("  param[%d] name:%s type:%d, flags:%d\n",
		       param->pos, param->name, param->type, param->flags);
	}
}

RPC_Request*
RPC_Request_new()
{
	RPC_Request* request = malloc(sizeof(RPC_Request));

	StringBuffer_init(&request->request_buffer, 1024);
	StringBuffer_init(&request->response_buffer, 1024);
	return request;
}

void
RPC_Request_free(RPC_Request* request)
{
	StringBuffer_free_members(&request->request_buffer);
	StringBuffer_free_members(&request->response_buffer);
	free(request);
}
