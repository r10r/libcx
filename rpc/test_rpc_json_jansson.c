#include "base/base.h"
#include "base/test.h"
#include "rpc_json.h"
#include "rpc_example_service.h"

#include <jansson.h>

static void
test_json_obj_param()
{
	Person person = {
		.firstname      = "Max",
		.lastname       = "Mustermann",
		.age            = 33
	};

	json_t* json = Person_to_json(&person);

	size_t flags = JSON_INDENT(2);
	char* json_string = json_dumps(json, flags);

	printf("%s", json_string);

	Person person2;
	memset(&person2, 0, sizeof(Person));

	Person_from_json(&person2, json);

	TEST_ASSERT_EQUAL_STRING(person.firstname, person2.firstname);
	TEST_ASSERT_EQUAL_STRING(person.lastname, person2.lastname);
	TEST_ASSERT_EQUAL_INT(person.age, person2.age);

	json_decref(json);
	cx_free(json_string);
}

static void
test_json_integer_to_params()
{
	json_t* json = json_pack("{s:i,s:I}", "int_val", INT_MAX, "long_val", INT_MAX + 1);

	Param* params = NULL;

	Params_from_json(&params, json);

	TEST_ASSERT_EQUAL_STRING("int_val", params[0].name);
	TEST_ASSERT_EQUAL_INT(-1, params[0].position);
	TEST_ASSERT_EQUAL(TYPE_INTEGER, params[0].value.type);
	TEST_ASSERT_EQUAL_INT(INT_MAX, params[0].value.value.integer);

	TEST_ASSERT_EQUAL_STRING("long_val", params[1].name);
	TEST_ASSERT_EQUAL_INT(-1, params[1].position);
	TEST_ASSERT_EQUAL(TYPE_LONGLONG, params[1].value.type);
	TEST_ASSERT_EQUAL_INT((long long)INT_MAX + 1, params[1].value.value.longlong);

	cx_free(params);
	json_decref(json);
}

static void
test_json_array_to_params()
{
	json_t* json = json_pack("[i,i,i]", 11, 22, 33);

	Param* params = NULL;

	Params_from_json(&params, json);

	TEST_ASSERT_EQUAL_STRING(NULL, params[0].name);
	TEST_ASSERT_EQUAL_INT(0, params[0].position);
	TEST_ASSERT_EQUAL(TYPE_INTEGER, params[0].value.type);
	TEST_ASSERT_EQUAL_INT(11, params[0].value.value.integer);

	TEST_ASSERT_EQUAL_STRING(NULL, params[1].name);
	TEST_ASSERT_EQUAL_INT(1, params[1].position);
	TEST_ASSERT_EQUAL(TYPE_INTEGER, params[1].value.type);
	TEST_ASSERT_EQUAL_INT(22, params[1].value.value.integer);

	TEST_ASSERT_EQUAL_STRING(NULL, params[2].name);
	TEST_ASSERT_EQUAL_INT(2, params[2].position);
	TEST_ASSERT_EQUAL(TYPE_INTEGER, params[2].value.type);
	TEST_ASSERT_EQUAL_INT(33, params[2].value.value.integer);

	cx_free(params);
	json_decref(json);
}

static void
test_call_print_person_json()
{
	Param params[1];

	memset(params, 0, sizeof(params));

	static Person person = {
		.firstname      = "Max",
		.lastname       = "Mustermann",
		.age            = 33
	};

	json_t* json = Person_to_json(&person);

	params[0].name = "person";
	params[0].position = 0;
	params[0].value.type = TYPE_OBJECT;
	params[0].value.value.object = json;
	params[0].value.f_free = (F_ValueFree*)&json_decref;

	RPC_Request request = {
		.method_name = "print_person",
		.params = params,
		.num_params = 1,
		.format = FORMAT_JSON
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
test_call_get_person_json()
{
	RPC_Request request = {
		.method_name = "get_person"
	};

	int status = Service_call(EXAMPLE_SERVICE_METHODS, &request);

	TEST_ASSERT_EQUAL_INT(1, status);
	TEST_ASSERT_EQUAL(TYPE_OBJECT, request.result.type);

	Person* person = (Person*)request.result.value.object;
	json_t* person_json = Value_to_json(&request.result);

	TEST_ASSERT_EQUAL(3, json_object_size(person_json));
	TEST_ASSERT_EQUAL_STRING(person->firstname, json_string_value(json_object_get(person_json, "firstname")));
	TEST_ASSERT_EQUAL_STRING(person->lastname, json_string_value(json_object_get(person_json, "lastname")));
	TEST_ASSERT_EQUAL_INT(person->age, json_integer_value(json_object_get(person_json, "age")));

	json_decref(person_json);
	if (request.result.f_free)
		request.result.f_free(request.result.value.object);
}

int
main()
{
	TEST_BEGIN

	RUN(test_json_integer_to_params);
	RUN(test_json_array_to_params);
	RUN(test_json_obj_param);
	RUN(test_call_print_person_json);
	RUN(test_call_get_person_json);

	TEST_END
}
