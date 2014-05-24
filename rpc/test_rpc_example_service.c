#include "base/base.h"
#include "base/test.h"
#include "rpc_json.h"
#include "rpc_example_service.h"

static void
test_call_error_method_missing()
{
	RPC_Request request = {
		.method_name = "unknown_method"
	};

	int status = Service_call(EXAMPLE_SERVICE_METHODS, &request);

	TEST_ASSERT_EQUAL_INT(-1, status);
	TEST_ASSERT_EQUAL_INT(RPC_ERROR_METHOD_MISSING, request.error);
}

static void
test_call_error_param_missing()
{
	RPC_Request request = {
		.method_name = "has_count"
	};

	int status = Service_call(EXAMPLE_SERVICE_METHODS, &request);

	TEST_ASSERT_EQUAL_INT(-1, status);
	TEST_ASSERT_EQUAL_INT(RPC_ERROR_PARAM_MISSING, request.error);
}

static void
test_call_error_param_missing2()
{
	Param params[1];

	memset(params, 0, sizeof(params));

	params[0].name = "string";
	params[0].value.type = TYPE_STRING;
	params[0].value.value.string = "foobar";

	RPC_Request request = {
		.method_name = "has_count",
		.params = params,
		.num_params = 1
	};

	int status = Service_call(EXAMPLE_SERVICE_METHODS, &request);

	TEST_ASSERT_EQUAL_INT(-1, status);
	TEST_ASSERT_EQUAL_INT(RPC_ERROR_PARAM_MISSING, request.error);
}

static void
test_call_error_param_type()
{
	Param params[1];

	memset(params, 0, sizeof(params));

	params[0].name = "string";
	params[0].value.type = TYPE_INTEGER;
	params[0].value.value.integer = 66;

	RPC_Request request = {
		.method_name = "has_count",
		.params = params,
		.num_params = 1
	};

	int status = Service_call(EXAMPLE_SERVICE_METHODS, &request);

	TEST_ASSERT_EQUAL_INT(-1, status);
	TEST_ASSERT_EQUAL_INT(RPC_ERROR_PARAM_INVALID_TYPE, request.error);
}

static void
test_call_has_count_true()
{
	Param params[2];

	memset(params, 0, sizeof(params));

	params[0].name = "string";
	params[0].value.type = TYPE_STRING;
	params[0].value.value.string = "foobar";

	params[1].name = "count";
	params[1].value.type = TYPE_INTEGER;
	params[1].value.value.integer = 6;

	RPC_Request request = {
		.method_name = "has_count",
		.params = params,
		.num_params = 2
	};

	int status = Service_call(EXAMPLE_SERVICE_METHODS, &request);

	TEST_ASSERT_EQUAL_INT(1, status);
	TEST_ASSERT_EQUAL_INT(0, request.error);
	TEST_ASSERT_EQUAL(TYPE_BOOLEAN, request.result.type);
	TEST_ASSERT_TRUE(request.result.value.boolean);

	if (request.result.f_free)
		request.result.f_free(request.result.value.object);
}

static void
test_call_has_count_false()
{
	Param params[2];

	memset(params, 0, sizeof(params));

	params[0].name = "string";
	params[0].value.type = TYPE_STRING;
	params[0].value.value.string = "foobar";

	params[1].name = "count";
	params[1].value.type = TYPE_INTEGER;
	params[1].value.value.integer = 1;

	RPC_Request request = {
		.method_name = "has_count",
		.params = params,
		.num_params = 2
	};

	int status = Service_call(EXAMPLE_SERVICE_METHODS, &request);

	TEST_ASSERT_EQUAL_INT(1, status);
	TEST_ASSERT_EQUAL_INT(0, request.error);
	TEST_ASSERT_EQUAL(TYPE_BOOLEAN, request.result.type);
	TEST_ASSERT_FALSE(request.result.value.boolean);

	if (request.result.f_free)
		request.result.f_free(request.result.value.object);
}

static void
test_call_has_count_error()
{
	Param params[2];

	memset(params, 0, sizeof(params));

	params[0].name = "string";
	params[0].value.type = TYPE_STRING;
	params[0].value.value.string = NULL;

	params[1].name = "count";
	params[1].value.type = TYPE_INTEGER;
	params[1].value.value.integer = 1;

	RPC_Request request = {
		.method_name = "has_count",
		.params = params,
		.num_params = 2
	};

	int status = Service_call(EXAMPLE_SERVICE_METHODS, &request);

	TEST_ASSERT_EQUAL_INT(-1, status);
	TEST_ASSERT_EQUAL_INT(RPC_ERROR_PARAM_NULL, request.error);

	if (request.result.f_free)
		request.result.f_free(request.result.value.object);
}

static void
test_call_print_person()
{
	Param params[1];

	memset(params, 0, sizeof(params));

	Person person = {
		.firstname      = "Max",
		.lastname       = "Mustermann",
		.age            = 33
	};

	params[0].name = "person";
	params[0].position = 0;
	params[0].value.type = TYPE_OBJECT;
	params[0].value.value.object = &person;

	RPC_Request request = {
		.method_name = "print_person",
		.params = params,
		.num_params = 1
	};

	int status = Service_call(EXAMPLE_SERVICE_METHODS, &request);

	TEST_ASSERT_EQUAL_INT(0, status);
	TEST_ASSERT_EQUAL_INT(0, request.error);
	TEST_ASSERT_EQUAL(TYPE_STRING, request.result.type);
	TEST_ASSERT_EQUAL_STRING("Max Mustermann (age 33)", request.result.value.string);

	if (request.result.f_free)
		request.result.f_free(request.result.value.object);
}

static void
test_call_get_person()
{
	RPC_Request request = {
		.method_name = "get_person"
	};

	int status = Service_call(EXAMPLE_SERVICE_METHODS, &request);

	TEST_ASSERT_EQUAL_INT(1, status);
	TEST_ASSERT_EQUAL(TYPE_OBJECT, request.result.type);

	if (request.result.f_free)
		request.result.f_free(request.result.value.object);
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
