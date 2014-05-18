#include "base/base.h"
#include "base/test.h"
#include "rpc_json.h"
#include "rpc_example_service.h"

static void
test_call_error_method_missing()
{
	int status = ExampleService_call("unknown_method", NULL, 0, NULL, FORMAT_NATIVE);

	TEST_ASSERT_EQUAL_INT(-1, status);
	TEST_ASSERT_EQUAL_INT(ERROR_METHOD_MISSING, cx_errno);
}

static void
test_call_error_param_missing()
{
	int status = ExampleService_call("has_count", NULL, 0, NULL, FORMAT_NATIVE);

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

	int status = ExampleService_call("has_count", params, 1, NULL, FORMAT_NATIVE);

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

	int status = ExampleService_call("has_count", params, 1, NULL, FORMAT_NATIVE);

	TEST_ASSERT_EQUAL_INT(-1, status);
	TEST_ASSERT_EQUAL_INT(ERROR_PARAM_TYPE, cx_errno);
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

	int status = ExampleService_call("has_count", params, 2, &result, FORMAT_NATIVE);

	TEST_ASSERT_EQUAL_INT(1, status);
	TEST_ASSERT_EQUAL_INT(0, cx_errno);
	TEST_ASSERT_EQUAL(TYPE_BOOLEAN, result.type);
	TEST_ASSERT_TRUE(result.value.boolean);
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

	int status = ExampleService_call("has_count", params, 2, &result, FORMAT_NATIVE);

	TEST_ASSERT_EQUAL_INT(1, status);
	TEST_ASSERT_EQUAL_INT(0, cx_errno);
	TEST_ASSERT_EQUAL(TYPE_BOOLEAN, result.type);
	TEST_ASSERT_FALSE(result.value.boolean);
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

	int status = ExampleService_call("has_count", params, 2, &result, FORMAT_NATIVE);

	TEST_ASSERT_EQUAL_INT(-1, status);
	TEST_ASSERT_EQUAL_INT(ERROR_PARAM_NULL, cx_errno);
}

static void
test_call_print_person()
{
	Param params[1];

	memset(params, 0, sizeof(params));

	Person person = {
		.firstname	= "Max",
		.lastname	= "Mustermann",
		.age		= 33
	};

	params[0].name = "person";
	params[0].position = 0;
	params[0].value.type = TYPE_OBJECT;
	params[0].value.value.object = &person;

	Value result;
	memset(&result, 0, sizeof(Value));
	int status = ExampleService_call("print_person", params, 2, &result, FORMAT_NATIVE);

	TEST_ASSERT_EQUAL_INT(0, status);
	TEST_ASSERT_EQUAL_INT(0, cx_errno);
	TEST_ASSERT_EQUAL(TYPE_STRING, result.type);
	TEST_ASSERT_EQUAL_STRING("Max Mustermann (age 33)", result.value.string);

	result.f_free(result.value.object);
}

static void
test_call_get_person()
{
	Value result;

	memset(&result, 0, sizeof(result));

	int status = ExampleService_call("get_person", NULL, 0, &result, FORMAT_NATIVE);

	TEST_ASSERT_EQUAL_INT(1, status);
	TEST_ASSERT_EQUAL(TYPE_OBJECT, result.type);

//	json_t* json = result.f_to_json(result.value.object);
	result.f_free(result.value.object); /* now we can free the result */
//
//	char* json_string = json_dumps(json, 0);
//	const char* expected_json = "{\"firstname\": \"Max\", \"lastname\": \"Mustermann\", \"age\": 33}";
//	TEST_ASSERT_EQUAL_STRING(expected_json, json_string);
//
//	cx_free(json_string);
//	json_decref(json);
}

static void
test_call_print_person_json()
{
	Param params[1];

	memset(params, 0, sizeof(params));

	Person person = {
		.firstname	= "Max",
		.lastname	= "Mustermann",
		.age		= 33
	};

	/*
	 * I know the type object when getting the json.
	 * The service wrapper knows which method to call,
	 * it just has to know the format
	 *
	 *  params are deserialized in the serivce layer
	 */

	params[0].name = "person";
	params[0].position = 0;
	params[0].value.type = TYPE_OBJECT;
	params[0].value.value.object = &person;


	Value result;
	memset(&result, 0, sizeof(Value));

	int status = ExampleService_call("print_person", params, 2, &result, FORMAT_JSON);

	TEST_ASSERT_EQUAL_INT(0, status);
	TEST_ASSERT_EQUAL_INT(0, cx_errno);
	TEST_ASSERT_EQUAL(TYPE_STRING, result.type);
	TEST_ASSERT_EQUAL_STRING("Max Mustermann (age 33)", result.value.string);

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
//	RUN(test_call_print_person_json);

	TEST_END
}
