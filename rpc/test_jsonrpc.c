#include "base/test.h"
#include "jsrpc.h"
#include "echo_service.h"

static void
assert_response_equals(RPC_Method methods[], StringBuffer* request, StringBuffer* response)
{
	RPC_RequestList* request_list = RPC_RequestList_new();

	StringBuffer_cat(request_list->request_buffer, StringBuffer_value(request));

	RPC_RequestList_process(request_list, methods);

	TEST_ASSERT_EQUAL_STRING(StringBuffer_value(response),
				 StringBuffer_value(request_list->response_buffer));

	RPC_RequestList_free(request_list);
	StringBuffer_free(request);
	StringBuffer_free(response);
}

static void
test_error_parse_error()
{
	RPC_Method echo_methods[] = { RPC_methods(Echo), RPC_Method_none };

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
	RPC_Method echo_methods[] = { RPC_methods(Echo), RPC_Method_none };

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
	RPC_Method echo_methods[] = { RPC_methods(Echo), RPC_Method_none };

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
	RPC_Method echo_methods[] = { RPC_methods(Echo), RPC_Method_none };

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
	RPC_Method echo_methods[] = { RPC_methods(Echo), RPC_Method_none };

	// FIXME we have to parse all params in advance to detect missing params
	assert_response_equals(
		echo_methods,
		StringBuffer_from_printf(1024, JSONRPC_REQUEST_SIMPLE, "1234", "echo"),
		StringBuffer_from_printf(1024, JSONRPC_RESPONSE_ERROR, "1234",
					 jsrpc_ERROR_INVALID_PARAMS, jsrpc_strerror(jsrpc_ERROR_INVALID_PARAMS))
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
//	RUN(test_invalid_params);

	TEST_END
}
