#include "jsrpc.h"

static List* methods;

static RPC_Method*
RPC_Method_new(const char* name, F_RPCMethod* fmethod, jsrpc_Param signature[], int param_count)
{
	RPC_Method* m = malloc(sizeof(RPC_Method));

	m->name = strdup(name);
	m->method = fmethod;
	m->signature = signature;
	m->param_count = param_count;
	return m;
}

static void
RPC_Method_free(RPC_Method* method)
{
	free(method->name);
	free(method);
}

static void
RPC_Method_log(RPC_Method* method)
{
	printf("Register method [%s] params:#%d\n", method->name, method->param_count);
	int i = 0;
	for (i = 0; i < method->param_count; i++)
		printf("  param[%d] %s flags:%d\n", i,
		       method->signature[i].name, method->signature[i].flags);
}

void
register_method(const char* name, F_RPCMethod* method, jsrpc_Param signature[], int param_count)
{
	RPC_Method* m = RPC_Method_new(name, method, signature, param_count);

	RPC_Method_log(m);
	List_push(methods, m);
}

static void
hello(const char* name)
{
	printf("Hello %s\n", name);
}

// should only be initialized once
// index in signature array = parameter index if name is null
// this way every method supports either parameters by position or name
METHOD_SIGNATURE(rpc_hello)
{
	{ "name", 0, get_param_value_string }
};

static void*
RPC_Request_get_param(RPC_Request* request, const char* name)
{
	return NULL;
}

static void
rpc_hello(RPC_Request* request)
{
	printf("rpc_hello: %s\n", (char*)request->params[0]);
	const char* name = RPC_Request_get_param(request, "name");

	hello((char*)request->params[0]);
}

static const char* const JSONRPC_REQUEST =
	JSONRPC_RESPONSE_HEADER "\"method\":\"%s\",\"params\":{\"%s\":\"%s\"}}";

static const char* const JSONRPC_REQUEST_POS =
	JSONRPC_RESPONSE_HEADER "\"method\":\"%s\",\"params\":[\"%s\"]}";

int
main()
{
	methods  = List_new();
	REGISTER_METHOD(rpc_hello);

	/* we have the whole config file in memory.  let's parse it ... */
	char request[1024];

	snprintf(request, sizeof(request), JSONRPC_REQUEST, "\"foobar\"", "rpc_hello", "name", "World");
	dispatch_request(methods, request);

	snprintf(request, sizeof(request), JSONRPC_REQUEST_POS, "\"foobar\"", "rpc_hello", "World");
	dispatch_request(methods, request);

	// read input from stdin parse and dispatch to method
}
