#include "base/base.h"
#include "base/test.h"
#include "rpc_json.h"
#include "rpc_example_service.h"

static void
test_call_error_method_missing()
{
	int status = Service_call(EXAMPLE_SERVICE_METHODS, "unknown_method", NULL, 0, NULL, FORMAT_NATIVE);

	TEST_ASSERT_EQUAL_INT(-1, status);
	TEST_ASSERT_EQUAL_INT(ERROR_METHOD_MISSING, cx_errno);
}

static void
test_call_error_param_missing()
{
	int status = Service_call(EXAMPLE_SERVICE_METHODS, "has_count", NULL, 0, NULL, FORMAT_NATIVE);

	TEST_ASSERT_EQUAL_INT(-1, status);
	TEST_ASSERT_EQUAL_INT(ERROR_PARAM_MISSING, cx_errno);
}

static void
test_call_error_param_missing2()
{
	Param params[1];

	memset(params, 0, sizeof(params));

	params[0].name = "string";
	params[0].value.type = TYPE_STRING;
	params[0].value.value.string = "foobar";

	int status = Service_call(EXAMPLE_SERVICE_METHODS, "has_count", params, 1, NULL, FORMAT_NATIVE);

	TEST_ASSERT_EQUAL_INT(-1, status);
	TEST_ASSERT_EQUAL_INT(ERROR_PARAM_MISSING, cx_errno);
}

static void
test_call_error_param_type()
{
	Param params[1];

	memset(params, 0, sizeof(params));

	params[0].name = "string";
	params[0].value.type = TYPE_INTEGER;
	params[0].value.value.integer = 66;

	int status = Service_call(EXAMPLE_SERVICE_METHODS, "has_count", params, 1, NULL, FORMAT_NATIVE);

	TEST_ASSERT_EQUAL_INT(-1, status);
	TEST_ASSERT_EQUAL_INT(ERROR_PARAM_INVALID_TYPE, cx_errno);
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

	Value result;
	memset(&result, 0, sizeof(result));

	int status = Service_call(EXAMPLE_SERVICE_METHODS, "has_count", params, 2, &result, FORMAT_NATIVE);

	TEST_ASSERT_EQUAL_INT(1, status);
	TEST_ASSERT_EQUAL_INT(0, cx_errno);
	TEST_ASSERT_EQUAL(TYPE_BOOLEAN, result.type);
	TEST_ASSERT_TRUE(result.value.boolean);

	if (result.f_free)
		result.f_free(result.value.object);
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

	Value result;
	memset(&result, 0, sizeof(result));

	int status = Service_call(EXAMPLE_SERVICE_METHODS, "has_count", params, 2, &result, FORMAT_NATIVE);

	TEST_ASSERT_EQUAL_INT(1, status);
	TEST_ASSERT_EQUAL_INT(0, cx_errno);
	TEST_ASSERT_EQUAL(TYPE_BOOLEAN, result.type);
	TEST_ASSERT_FALSE(result.value.boolean);

	if (result.f_free)
		result.f_free(result.value.object);
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

	Value result;
	memset(&result, 0, sizeof(result));

	int status = Service_call(EXAMPLE_SERVICE_METHODS, "has_count", params, 2, &result, FORMAT_NATIVE);

	TEST_ASSERT_EQUAL_INT(-1, status);
	TEST_ASSERT_EQUAL_INT(ERROR_PARAM_NULL, cx_errno);

	if (result.f_free)
		result.f_free(result.value.object);
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

	Value result;
	memset(&result, 0, sizeof(Value));
	int status = Service_call(EXAMPLE_SERVICE_METHODS, "print_person", params, 1, &result, FORMAT_NATIVE);

	TEST_ASSERT_EQUAL_INT(0, status);
	TEST_ASSERT_EQUAL_INT(0, cx_errno);
	TEST_ASSERT_EQUAL(TYPE_STRING, result.type);
	TEST_ASSERT_EQUAL_STRING("Max Mustermann (age 33)", result.value.string);

	if (result.f_free)
		result.f_free(result.value.object);
}

static void
test_call_get_person()
{
	Value result;

	memset(&result, 0, sizeof(result));

	int status = Service_call(EXAMPLE_SERVICE_METHODS, "get_person", NULL, 0, &result, FORMAT_NATIVE);

	TEST_ASSERT_EQUAL_INT(1, status);
	TEST_ASSERT_EQUAL(TYPE_OBJECT, result.type);

	if (result.f_free)
		result.f_free(result.value.object);
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
