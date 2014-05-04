#include "rpc.h"

const RPC_Method RPC_null =  { NULL, NULL, NULL, 0 };

RPC_Method*
RPC_Method_new(const char* name, F_RPC_Method* fmethod, RPC_Param signature[], int param_count)
{
	RPC_Method* m = malloc(sizeof(RPC_Method));

	m->name = strdup(name);
	m->method = fmethod;
	m->signature = signature;
	m->param_count = param_count;
	return m;
}

void
RPC_Method_free(RPC_Method* method)
{
	free(method->name);
	free(method);
}

void
RPC_Method_log(RPC_Method* method)
{
	printf("* method[%s] params:#%d\n", method->name, method->param_count);
	int i = 0;
	for (i = 0; i < method->param_count; i++)
		printf("  param[%d] name:%s flags:%d\n", i,
		       method->signature[i].name, method->signature[i].flags);
}