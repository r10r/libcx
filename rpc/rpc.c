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
	XDBG("Registered RPC methods:");
	XFDBG("* method[%s] %p params:#%d", method->name, (unsigned char*)&method->method, method->param_count);
	int i = 0;
	for (i = 0; i < method->param_count; i++)
	{
		RPC_Param* param = method->signature[i];
		UNUSED(param);
		XFDBG("  param[%d] name:%s type:%d, flags:%d",
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

RPC_RequestList*
RPC_RequestList_new(size_t buffer_size)
{
	RPC_RequestList* request_list = cx_alloc(sizeof(RPC_RequestList));

	request_list->request_buffer = StringBuffer_new(buffer_size);
	request_list->response_buffer = StringBuffer_new(buffer_size);
	request_list->result_buffer = StringBuffer_new(buffer_size);
	request_list->response_list = List_new();

	return request_list;
}

// TODO recycle request list with buffers for next request (pooling)
void
RPC_RequestList_free(RPC_RequestList* request_list)
{
	RPC_RequestList_free_data(request_list);
	cx_free(request_list->requests);
	StringBuffer_free(request_list->result_buffer);
	StringBuffer_free(request_list->request_buffer);
	StringBuffer_free(request_list->response_buffer);
	List_free(request_list->response_list);

	cx_free(request_list);
}
