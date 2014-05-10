#include "base/test.h"
#include "jsrpc.h"

static void
test_jsrpc_printf()
{
	printf("%s\n", JSONRPC_RESPONSE);
	printf("%s\n", JSONRPC_RESPONSE_STRING);
	printf("%s\n", JSONRPC_ERROR);
	jsrpc_fprintf_response(stdout, "22", "foobar");
	jsrpc_fprintf_error(stdout, NULL, jsrpc_ERROR_INVALID_REQUEST, "Invalid request");
}

static void
test_write_object()
{
	StringBuffer* buffer = StringBuffer_new(1024);

	// \0 terminator belongs to string length or not ?
	// are strings always null terminated or not

	StringBuffer_cat(buffer, "{");                                  /* add null terminator, but does not overwrite it */
	StringBuffer_aprintf(buffer, JSRPC_STRINGPAIR("foo"), "bar");   /* adds null terminator, overwrites null terminator */
	StringBuffer_cat(buffer, "}");

	printf("%s\n", StringBuffer_value(buffer));
	TEST_ASSERT_EQUAL_STRING("{\"foo\":\"bar\"}", StringBuffer_value(buffer));

	StringBuffer_free(buffer);
}

int
main()
{
	TEST_BEGIN

	RUN(test_jsrpc_printf);
	RUN(test_write_object);

	TEST_END
}
