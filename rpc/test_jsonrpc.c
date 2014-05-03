#include "base/test.h"
#include "jsrpc.h"

static void
test_format()
{
	printf("%s", JSONRPC_RESPONSE);
	printf("%s", JSONRPC_RESPONSE_STRING);
	printf("%s", JSONRPC_ERROR);
	jsrpc_fprintf_response(stdout, "22", "foobar");
	jsrpc_fprintf_error(stdout, NULL, jsrpc_ERROR_INVALID_REQUEST, "Invalid request");
}

void
register_method(const char* method, F_RPCMethod* fmethod, jsrpc_Param signature[], int param_count)
{
	printf("Register %s params #%d\n", method, param_count);
	int i = 0;
	for (i = 0; i < param_count; i++)
		printf("  param[%d] %s\n", i, signature[i].name);
}

// should only be initialized once
// index in signature array = parameter index if name is null
// this way every method supports either parameters by position or name
METHOD_SIGNATURE(test_foo)
{
	{ "divident", 0, NULL },
	{ "divisor", 0, NULL }
};
static void
test_foo(RPC_Request* request)
{
	// error handling for parameters is done in router (invalid parameters, invalid request ...)

	// error handling for method call is done in the RPC method
}

static void
test_divide()
{
	REGISTER_METHOD(test_foo);
}

int
main()
{
	TEST_BEGIN

	RUN(test_format);
	RUN(test_divide);

	TEST_END
}
