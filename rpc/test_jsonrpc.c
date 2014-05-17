#include "base/test.h"
#include "echo_service.h"

static void
assert_response_equals(RPC_Method methods[], StringBuffer* request_buffer, StringBuffer* expected_response_buffer)
{
	RPC_RequestList* request_list = RPC_RequestList_new(Request_new(request_buffer), StringBuffer_value(request_buffer), 2048);

	RPC_RequestList_process(request_list, methods);

	TEST_ASSERT_EQUAL_STRING(StringBuffer_value(expected_response_buffer),
				 StringBuffer_value(request_list->response_buffer));

	RPC_RequestList_free(request_list);
	StringBuffer_free(expected_response_buffer);
	StringBuffer_free(request_buffer);
}

static void
test_error_parse_error()
{
	RPC_Method echo_methods[] = { EchoService_methods, RPC_Method_none };

	assert_response_equals(
		echo_methods,
		StringBuffer_from_string(S_dup("foobar")),
		StringBuffer_from_printf(1024, JSONRPC_RESPONSE_ERROR, "null",
					 jsrpc_ERROR_PARSE_ERROR, jsrpc_strerror(jsrpc_ERROR_PARSE_ERROR))
		);

	assert_response_equals(
		echo_methods,
		StringBuffer_from_string(S_dup("")),
		StringBuffer_from_printf(1024, JSONRPC_RESPONSE_ERROR, "null",
					 jsrpc_ERROR_PARSE_ERROR, jsrpc_strerror(jsrpc_ERROR_PARSE_ERROR))
		);
}

static void
test_error_invalid_request()
{
	RPC_Method echo_methods[] = { EchoService_methods, RPC_Method_none };

	assert_response_equals(
		echo_methods,
		StringBuffer_from_string(S_dup("{}")),
		StringBuffer_from_printf(1024, JSONRPC_RESPONSE_ERROR, "null",
					 jsrpc_ERROR_INVALID_REQUEST, jsrpc_strerror(jsrpc_ERROR_INVALID_REQUEST))
		);

	assert_response_equals(
		echo_methods,
		StringBuffer_from_string(S_dup("[]")),
		StringBuffer_from_printf(1024, JSONRPC_RESPONSE_ERROR, "null",
					 jsrpc_ERROR_INVALID_REQUEST, jsrpc_strerror(jsrpc_ERROR_INVALID_REQUEST))
		);
}

static void
test_error_method_not_found()
{
	RPC_Method echo_methods[] = { EchoService_methods, RPC_Method_none };

	assert_response_equals(
		echo_methods,
		StringBuffer_from_printf(1024, JSONRPC_REQUEST_SIMPLE, "1234", "fooobar"),
		StringBuffer_from_printf(1024, JSONRPC_RESPONSE_ERROR, "1234",
					 jsrpc_ERROR_METHOD_NOT_FOUND, jsrpc_strerror(jsrpc_ERROR_METHOD_NOT_FOUND))
		);
}

static void
test_error_invalid_batch_request()
{
	RPC_Method echo_methods[] = { EchoService_methods, RPC_Method_none };

	StringBuffer* error = StringBuffer_from_printf(1024, JSONRPC_RESPONSE_ERROR, JSONRPC_NULL,
						       jsrpc_ERROR_INVALID_REQUEST, jsrpc_strerror(jsrpc_ERROR_INVALID_REQUEST));

	StringBuffer* invalid_batch_response = StringBuffer_from_printf(2048, "[%s,%s,%s]",
									StringBuffer_value(error), StringBuffer_value(error), StringBuffer_value(error));

	StringBuffer_free(error);

	assert_response_equals(
		echo_methods,
		StringBuffer_from_string(S_dup("[1,2,3]")),
		invalid_batch_response
		);
}

static void
test_invalid_params()
{
	RPC_Method echo_methods[] = { EchoService_methods, RPC_Method_none };

	assert_response_equals(
		echo_methods,
		StringBuffer_from_printf(1024, JSONRPC_REQUEST_SIMPLE, "1234", "echo"),
		StringBuffer_from_printf(1024, JSONRPC_RESPONSE_ERROR, "1234",
					 jsrpc_ERROR_INVALID_PARAMS, jsrpc_strerror(jsrpc_ERROR_INVALID_PARAMS))
		);

	assert_response_equals(
		echo_methods,
		StringBuffer_from_printf(1024, JSONRPC_REQUEST, "1234", "echo", "[1]"),
		StringBuffer_from_printf(1024, JSONRPC_RESPONSE_ERROR, "1234",
					 jsrpc_ERROR_INVALID_PARAMS, jsrpc_strerror(jsrpc_ERROR_INVALID_PARAMS))
		);
}

static void
test_valid_params()
{
	RPC_Method echo_methods[] = { EchoService_methods, RPC_Method_none };

	/* position parameter */
	assert_response_equals(
		echo_methods,
		StringBuffer_from_printf(1024, JSONRPC_REQUEST, "1234", "echo", "[\"hello\"]"),
		StringBuffer_from_printf(1024, JSONRPC_RESPONSE_STRING, "1234", "hello")
		);

	/* named based parameter */
	assert_response_equals(
		echo_methods,
		StringBuffer_from_printf(1024, JSONRPC_REQUEST, "1234", "echo", "{\"input\":\"hello\"}"),
		StringBuffer_from_printf(1024, JSONRPC_RESPONSE_STRING, "1234", "hello")
		);
}

static void
test_batch_invalid()
{
	RPC_Method echo_methods[] = { EchoService_methods, RPC_Method_none };

	/* build expected response */
	StringBuffer* batch_response = StringBuffer_new(2048);

	StringBuffer_cat(batch_response, "[");
	StringBuffer_aprintf(batch_response, JSONRPC_RESPONSE_ERROR, JSONRPC_NULL,
			     jsrpc_ERROR_INVALID_REQUEST, jsrpc_strerror(jsrpc_ERROR_INVALID_REQUEST));
	StringBuffer_cat(batch_response, "]");

	assert_response_equals(
		echo_methods,
		StringBuffer_from_string(S_dup("[1]")),
		batch_response
		);
}

static void
test_batch_single()
{
	RPC_Method echo_methods[] = { EchoService_methods, RPC_Method_none };
	StringBuffer* batch_request = StringBuffer_new(2048);
	StringBuffer* batch_response = StringBuffer_new(2048);

	// StringBuffer_cat(batch_response, ",");

	/* build request */
	StringBuffer_cat(batch_request, "[");
	StringBuffer_aprintf(batch_request, JSONRPC_REQUEST_SIMPLE, "1234", "echo"),
	StringBuffer_cat(batch_request, "]");

	/* build expected response */
	StringBuffer_cat(batch_response, "[");
	StringBuffer_aprintf(batch_response, JSONRPC_RESPONSE_ERROR, "1234",
			     jsrpc_ERROR_INVALID_PARAMS, jsrpc_strerror(jsrpc_ERROR_INVALID_PARAMS));
	StringBuffer_cat(batch_response, "]");

	assert_response_equals(
		echo_methods,
		batch_request,
		batch_response
		);
}

static void
test_batch_multi()
{
	RPC_Method echo_methods[] = { EchoService_methods, RPC_Method_none };
	StringBuffer* batch_request = StringBuffer_new(2048);
	StringBuffer* batch_response = StringBuffer_new(2048);

	// StringBuffer_cat(batch_response, ",");

	/* build request */
	StringBuffer_cat(batch_request, "[");
	StringBuffer_aprintf(batch_request, JSONRPC_REQUEST, "1234", "echo", "[\"hello\"]"),
	StringBuffer_cat(batch_request, ",{},");        // invalid request
	StringBuffer_aprintf(batch_request, JSONRPC_REQUEST_SIMPLE, "\"myid\"", "fooobar"),
	StringBuffer_cat(batch_request, "]");

	/* build expected response */
	StringBuffer_cat(batch_response, "[");
	StringBuffer_aprintf(batch_response, JSONRPC_RESPONSE_STRING, "1234", "hello");
	StringBuffer_cat(batch_response, ",");
	StringBuffer_aprintf(batch_response, JSONRPC_RESPONSE_ERROR, JSONRPC_NULL,
			     jsrpc_ERROR_INVALID_REQUEST, jsrpc_strerror(jsrpc_ERROR_INVALID_REQUEST));
	StringBuffer_cat(batch_response, ",");
	StringBuffer_aprintf(batch_response, JSONRPC_RESPONSE_ERROR, "myid",
			     jsrpc_ERROR_METHOD_NOT_FOUND, jsrpc_strerror(jsrpc_ERROR_METHOD_NOT_FOUND));
	StringBuffer_cat(batch_response, "]");

	assert_response_equals(
		echo_methods,
		batch_request,
		batch_response
		);
}

int
main()
{
	TEST_BEGIN

	RUN(test_error_parse_error);
	RUN(test_error_invalid_request);
	RUN(test_error_method_not_found);
	RUN(test_error_invalid_batch_request);
	RUN(test_invalid_params);
	RUN(test_valid_params);
	RUN(test_batch_invalid);
	RUN(test_batch_single);
	RUN(test_batch_multi);

	TEST_END
}
