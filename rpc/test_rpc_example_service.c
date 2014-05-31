#include <libcx/base/base.h>
#include <libcx/base/test.h>

#include "rpc_json.h"
#include "rpc_example_service.h"

/*
 * PARAMS are not malloced in the tests
 */
static void
RPC_Request_free_ignore_params(RPC_Request* request)
{
	if (request->result.value.f_free)
		request->result.value.f_free(request->result.value.data.object);
}

static void
test_call_error_method_missing()
{
	RPC_Request request = {
		.method_name = "unknown_method"
	};

	Service_call(EXAMPLE_SERVICE_METHODS, &request);

	TEST_ASSERT_EQUAL_INT(CX_RPC_ERROR_METHOD_NOT_FOUND, request.result.error);

	RPC_Request_free_ignore_params(&request);
}

static void
test_call_error_param_missing()
{
	RPC_Request request = {
		.method_name = "has_count"
	};

	Service_call(EXAMPLE_SERVICE_METHODS, &request);

	TEST_ASSERT_EQUAL_INT(CX_RPC_ERROR_INVALID_PARAMS, request.result.error);

	RPC_Request_free_ignore_params(&request);
}

static void
test_call_error_param_missing2()
{
	RPC_Param params[1];

	memset(params, 0, sizeof(params));

	params[0].name = "string";
	params[0].value.type = RPC_TYPE_STRING;
	params[0].value.data.string = "foobar";

	RPC_Request request = {
		.method_name = "has_count",
		.params = params,
		.num_params = 1
	};

	Service_call(EXAMPLE_SERVICE_METHODS, &request);

	TEST_ASSERT_EQUAL_INT(CX_RPC_ERROR_INVALID_PARAMS, request.result.error);

	RPC_Request_free_ignore_params(&request);
}

static void
test_call_error_param_type()
{
	RPC_Param params[1];

	memset(params, 0, sizeof(params));

	params[0].name = "string";
	params[0].value.type = RPC_TYPE_INTEGER;
	params[0].value.data.integer = 66;

	RPC_Request request = {
		.method_name = "has_count",
		.params = params,
		.num_params = 1
	};

	Service_call(EXAMPLE_SERVICE_METHODS, &request);

	TEST_ASSERT_EQUAL_INT(CX_RPC_ERROR_INVALID_PARAMS, request.result.error);

	RPC_Request_free_ignore_params(&request);
}

static void
test_call_has_count_true()
{
	RPC_Param params[2];

	memset(params, 0, sizeof(params));

	params[0].name = "string";
	params[0].value.type = RPC_TYPE_STRING;
	params[0].value.data.string = "foobar";

	params[1].name = "count";
	params[1].value.type = RPC_TYPE_INTEGER;
	params[1].value.data.integer = 6;

	RPC_Request request = {
		.method_name = "has_count",
		.params = params,
		.num_params = 2
	};

	Service_call(EXAMPLE_SERVICE_METHODS, &request);

	TEST_ASSERT_EQUAL_INT(CX_ERR_OK, request.result.error);
	TEST_ASSERT_EQUAL(RPC_TYPE_BOOLEAN, request.result.value.type);
	TEST_ASSERT_TRUE(request.result.value.data.boolean);

	RPC_Request_free_ignore_params(&request);
}

static void
test_call_has_count_false()
{
	RPC_Param params[2];

	memset(params, 0, sizeof(params));

	params[0].name = "string";
	params[0].value.type = RPC_TYPE_STRING;
	params[0].value.data.string = "foobar";

	params[1].name = "count";
	params[1].value.type = RPC_TYPE_INTEGER;
	params[1].value.data.integer = 1;

	RPC_Request request = {
		.method_name = "has_count",
		.params = params,
		.num_params = 2
	};

	Service_call(EXAMPLE_SERVICE_METHODS, &request);

	TEST_ASSERT_EQUAL_INT(CX_ERR_OK, request.result.error);
	TEST_ASSERT_EQUAL(RPC_TYPE_BOOLEAN, request.result.value.type);
	TEST_ASSERT_FALSE(request.result.value.data.boolean);

	RPC_Request_free_ignore_params(&request);
}

static void
test_call_has_count_error()
{
	RPC_Param params[2];

	memset(params, 0, sizeof(params));

	params[0].name = "string";
	params[0].value.type = RPC_TYPE_STRING;
	params[0].value.data.string = NULL;

	params[1].name = "count";
	params[1].value.type = RPC_TYPE_INTEGER;
	params[1].value.data.integer = 1;

	RPC_Request request = {
		.method_name = "has_count",
		.params = params,
		.num_params = 2
	};

	Service_call(EXAMPLE_SERVICE_METHODS, &request);

	TEST_ASSERT_EQUAL_INT(CX_RPC_ERROR_INVALID_PARAMS, request.result.error);

	RPC_Request_free_ignore_params(&request);
}

static void
test_call_print_person()
{
	RPC_Param params[1];

	memset(params, 0, sizeof(params));

	Person person = {
		.firstname      = "Max",
		.lastname       = "Mustermann",
		.age            = 33
	};

	params[0].name = "person";
	params[0].position = 0;
	params[0].value.type = RPC_TYPE_OBJECT;
	params[0].value.data.object = &person;

	RPC_Request request = {
		.method_name = "print_person",
		.params = params,
		.num_params = 1
	};

	Service_call(EXAMPLE_SERVICE_METHODS, &request);

	TEST_ASSERT_EQUAL_INT(CX_ERR_OK, request.result.error);
	TEST_ASSERT_EQUAL(RPC_TYPE_STRING, request.result.value.type);
	TEST_ASSERT_EQUAL_STRING("Max Mustermann (age 33)", request.result.value.data.string);

	RPC_Request_free_ignore_params(&request);
}

static void
test_call_get_person()
{
	RPC_Request request = {
		.method_name = "get_person"
	};

	Service_call(EXAMPLE_SERVICE_METHODS, &request);

	TEST_ASSERT_EQUAL_INT(CX_ERR_OK, request.result.error);
	TEST_ASSERT_EQUAL(RPC_TYPE_OBJECT, request.result.value.type);

	RPC_Request_free_ignore_params(&request);
}

int
main()
{
	TEST_BEGIN

	RUN(test_call_error_method_missing);
	RUN(test_call_error_param_missing);
	RUN(test_call_error_param_missing2);
	RUN(test_call_error_param_type);
	RUN(test_call_has_count_true);
	RUN(test_call_has_count_false);
	RUN(test_call_has_count_error);
	RUN(test_call_print_person);
	RUN(test_call_get_person);

	TEST_END
}
