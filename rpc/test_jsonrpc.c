#include "base/test.h"
#include "jsrpc.h"

static void
test_jsrpc_printf()
{
	printf("%s", JSONRPC_RESPONSE);
	printf("%s", JSONRPC_RESPONSE_STRING);
	printf("%s", JSONRPC_ERROR);
	jsrpc_fprintf_response(stdout, "22", "foobar");
	jsrpc_fprintf_error(stdout, NULL, jsrpc_ERROR_INVALID_REQUEST, "Invalid request");
}

int
main()
{
	TEST_BEGIN

	RUN(test_jsrpc_printf);

	TEST_END
}
