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

	RPC_Param* params = NULL;

	Params_from_json(&params, json);

	TEST_ASSERT_EQUAL_STRING("int_val", params[0].name);
	TEST_ASSERT_EQUAL_INT(-1, params[0].position);
	TEST_ASSERT_EQUAL(RPC_TYPE_INTEGER, params[0].value.type);
	TEST_ASSERT_EQUAL_INT(INT_MAX, params[0].value.value.integer);

	TEST_ASSERT_EQUAL_STRING("long_val", params[1].name);
	TEST_ASSERT_EQUAL_INT(-1, params[1].position);
	TEST_ASSERT_EQUAL(RPC_TYPE_LONGLONG, params[1].value.type);
	TEST_ASSERT_EQUAL_INT((long long)INT_MAX + 1, params[1].value.value.longlong);

	cx_free(params);
	json_decref(json);
}

static void
test_json_array_to_params()
{
	json_t* json = json_pack("[i,i,i]", 11, 22, 33);

	RPC_Param* params = NULL;

	Params_from_json(&params, json);

	TEST_ASSERT_EQUAL_STRING(NULL, params[0].name);
	TEST_ASSERT_EQUAL_INT(0, params[0].position);
	TEST_ASSERT_EQUAL(RPC_TYPE_INTEGER, params[0].value.type);
	TEST_ASSERT_EQUAL_INT(11, params[0].value.value.integer);

	TEST_ASSERT_EQUAL_STRING(NULL, params[1].name);
	TEST_ASSERT_EQUAL_INT(1, params[1].position);
	TEST_ASSERT_EQUAL(RPC_TYPE_INTEGER, params[1].value.type);
	TEST_ASSERT_EQUAL_INT(22, params[1].value.value.integer);

	TEST_ASSERT_EQUAL_STRING(NULL, params[2].name);
	TEST_ASSERT_EQUAL_INT(2, params[2].position);
	TEST_ASSERT_EQUAL(RPC_TYPE_INTEGER, params[2].value.type);
	TEST_ASSERT_EQUAL_INT(33, params[2].value.value.integer);

	cx_free(params);
	json_decref(json);
}

static void
test_call_print_person_json()
{
	RPC_Param params[1];

	memset(params, 0, sizeof(params));

	static Person person = {
		.firstname      = "Max",
		.lastname       = "Mustermann",
		.age            = 33
	};

	json_t* json = Person_to_json(&person);

	params[0].name = "person";
	params[0].position = 0;
	params[0].value.type = RPC_TYPE_OBJECT;
	params[0].value.value.object = json;
	params[0].value.f_free = (F_RPC_ValueFree*)&json_decref;

	RPC_Request request = {
		.method_name = "print_person",
		.params = params,
		.num_params = 1,
		.format = FORMAT_JSON
	};

	int status = Service_call(EXAMPLE_SERVICE_METHODS, &request);

	TEST_ASSERT_EQUAL_INT(0, status);
	TEST_ASSERT_EQUAL_INT(0, request.error);
	TEST_ASSERT_EQUAL(RPC_TYPE_STRING, request.result.type);
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
	TEST_ASSERT_EQUAL(RPC_TYPE_OBJECT, request.result.type);

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

static void
test_deserialize_error_request_parse()
{
	const char* request_json = "\"jsonrpc\": \"2.0\", \"id\": 66, \"method\": \"play\"}";

	RPC_Request request;
	int status = Request_from_json(&request, request_json, strlen(request_json));

	TEST_ASSERT_EQUAL_INT(-1, status);
	TEST_ASSERT_EQUAL_INT(RPC_ERROR_REQUEST_PARSE, request.error);

	TEST_ASSERT_NULL(request.method_name);
	TEST_ASSERT_EQUAL_INT(RPC_ID_NONE, request.id_type);
	TEST_ASSERT_EQUAL_INT(0, request.id.number);
	TEST_ASSERT_EQUAL_INT(0, request.num_params);
	TEST_ASSERT_NULL(request.params);

	if (request.f_free)
		request.f_free(&request);
}

#define TEST_ASSERT_RPC_ERROR(json, expected_error) \
	{ \
		RPC_Request request; \
		int status = Request_from_json(&request, json, strlen(json)); \
\
		TEST_ASSERT_EQUAL_INT(-1, status); \
		TEST_ASSERT_EQUAL_INT(expected_error, request.error); \
\
		TEST_ASSERT_NULL(request.method_name); \
		TEST_ASSERT_EQUAL_INT(RPC_ID_NONE, request.id_type); \
		TEST_ASSERT_EQUAL_INT(0, request.id.number); \
		TEST_ASSERT_EQUAL_INT(0, request.num_params); \
		TEST_ASSERT_NULL(request.params); \
\
		if (request.f_free) \
			request.f_free(&request); \
	}

static void
test_deserialize_error_request_invalid()
{
	const char* request_json = "{\"id\": 66, \"method\": \"play\"}";

	TEST_ASSERT_RPC_ERROR(request_json, RPC_ERROR_REQUEST_INVALID);
}

static void
test_deserialize_error_request_invalid__jsonrpc_type()
{
	/* jsonrpc version value must be a string */
	const char* request_json = "{\"jsonrpc\": 2.0, \"id\": 66, \"method\": \"play\"}";

	TEST_ASSERT_RPC_ERROR(request_json, RPC_ERROR_REQUEST_INVALID);
}

static void
test_deserialize_error_request_invalid__missing_method_param()
{
	/* jsonrpc version value must be a string */
	const char* request_json = "{\"jsonrpc\": \"2.0\", \"id\": 66}";

	TEST_ASSERT_RPC_ERROR(request_json, RPC_ERROR_REQUEST_INVALID);
}

/* jsonrpc version value must be exactly '2.0' */
static void
test_deserialize_error_param_invalid_value__jsonrpc()
{
	const char* request_json = "{\"jsonrpc\": \"3.0\", \"id\": 66, \"method\": \"play\"}";

	TEST_ASSERT_RPC_ERROR(request_json, RPC_ERROR_PARAM_INVALID_VALUE);
}

/* jsonrpc version value must be a string or an integer*/
static void
test_deserialize_error_param_invalid_type__id()
{
	const char* request_json;

	request_json = "{\"jsonrpc\": \"2.0\", \"id\": null, \"method\": \"play\"}";

	TEST_ASSERT_RPC_ERROR(request_json, RPC_ERROR_PARAM_INVALID_TYPE);

	request_json = "{\"jsonrpc\": \"2.0\", \"id\": true, \"method\": \"play\"}";

	TEST_ASSERT_RPC_ERROR(request_json, RPC_ERROR_PARAM_INVALID_TYPE);

	request_json = "{\"jsonrpc\": \"2.0\", \"id\": {}, \"method\": \"play\"}";

	TEST_ASSERT_RPC_ERROR(request_json, RPC_ERROR_PARAM_INVALID_TYPE);

	request_json = "{\"jsonrpc\": \"2.0\", \"id\": [\"66\"], \"method\": \"play\"}";

	TEST_ASSERT_RPC_ERROR(request_json, RPC_ERROR_PARAM_INVALID_TYPE);
}

static void
test_deserialize_request__id()
{
	const char* request_json;
	RPC_Request request;
	int status;

	request_json = "{\"jsonrpc\": \"2.0\", \"id\": 66, \"method\": \"play\"}";
	status = Request_from_json(&request, request_json, strlen(request_json));

	TEST_ASSERT_EQUAL_INT(0, status);
	TEST_ASSERT_EQUAL_INT(RPC_ERROR_OK, request.error);
	TEST_ASSERT_EQUAL_STRING("play", request.method_name);
	TEST_ASSERT_EQUAL_INT(RPC_ID_NUMBER, request.id_type);
	TEST_ASSERT_EQUAL_INT(66, request.id.number);
	TEST_ASSERT_EQUAL_INT(0, request.num_params);
	TEST_ASSERT_NULL(request.params);

	request_json = "{\"jsonrpc\": \"2.0\", \"id\": \"66\", \"method\": \"play\"}";
	status = Request_from_json(&request, request_json, strlen(request_json));

	TEST_ASSERT_EQUAL_INT(0, status);
	TEST_ASSERT_EQUAL_INT(RPC_ERROR_OK, request.error);
	TEST_ASSERT_EQUAL_STRING("play", request.method_name);
	TEST_ASSERT_EQUAL_INT(RPC_ID_STRING, request.id_type);
	TEST_ASSERT_EQUAL_STRING("66", request.id.string);
	TEST_ASSERT_EQUAL_INT(0, request.num_params);
	TEST_ASSERT_NULL(request.params);

	request_json = "{\"jsonrpc\": \"2.0\", \"method\": \"play\"}";
	status = Request_from_json(&request, request_json, strlen(request_json));

	TEST_ASSERT_EQUAL_INT(0, status);
	TEST_ASSERT_EQUAL_INT(RPC_ERROR_OK, request.error);
	TEST_ASSERT_EQUAL_STRING("play", request.method_name);
	TEST_ASSERT_EQUAL_INT(RPC_ID_NONE, request.id_type);
	TEST_ASSERT_NULL(request.id.string);
	TEST_ASSERT_EQUAL_INT(0, request.num_params);
	TEST_ASSERT_NULL(request.params);
}

/* FIXME
 * - extraneous parameter should result in an error, otherwise the caller never notices ?
 * - maybe we have to add a strict mode ? (using jansson json_unpack_ex with JSON_STRICT ? )*/
static void
test_deserialize_request__extraneous_attributes()
{
	const char* request_json;
	RPC_Request request;
	int status;

	request_json = "{\"jsonrpc\": \"2.0\", \"id\": 66, \"method\": \"play\", \"foo\": \"bar\"}";
	status = Request_from_json(&request, request_json, strlen(request_json));

	TEST_ASSERT_EQUAL_INT(0, status);
	TEST_ASSERT_EQUAL_INT(RPC_ERROR_OK, request.error);
	TEST_ASSERT_EQUAL_STRING("play", request.method_name);
	TEST_ASSERT_EQUAL_INT(RPC_ID_NUMBER, request.id_type);
	TEST_ASSERT_EQUAL_INT(66, request.id.number);
	TEST_ASSERT_EQUAL_INT(0, request.num_params);
	TEST_ASSERT_NULL(request.params);
}

static void
test_deserialize_error_param_deserialize()
{
	const char* request_json;

	request_json = "{\"jsonrpc\": \"2.0\", \"method\": \"play\", \"params\" : 12345 }";

	TEST_ASSERT_RPC_ERROR(request_json, RPC_ERROR_PARAM_INVALID_TYPE);

	request_json = "{\"jsonrpc\": \"2.0\", \"method\": \"play\", \"params\" : 2.0 }";

	TEST_ASSERT_RPC_ERROR(request_json, RPC_ERROR_PARAM_INVALID_TYPE);

	request_json = "{\"jsonrpc\": \"2.0\", \"method\": \"play\", \"params\" : \"foobar\" }";

	TEST_ASSERT_RPC_ERROR(request_json, RPC_ERROR_PARAM_INVALID_TYPE);

	request_json = "{\"jsonrpc\": \"2.0\", \"method\": \"play\", \"params\" : null }";

	TEST_ASSERT_RPC_ERROR(request_json, RPC_ERROR_PARAM_INVALID_TYPE);

	request_json = "{\"jsonrpc\": \"2.0\", \"method\": \"play\", \"params\" : true }";

	TEST_ASSERT_RPC_ERROR(request_json, RPC_ERROR_PARAM_INVALID_TYPE);
}

static void
test_deserialize_request_empty_params()
{
	const char* request_json;
	RPC_Request request;
	int status;

	request_json = "{\"jsonrpc\": \"2.0\", \"method\": \"play\", \"params\" : []}";

	status = Request_from_json(&request, request_json, strlen(request_json));

	TEST_ASSERT_EQUAL_INT(0, status);
	TEST_ASSERT_EQUAL_INT(0, request.num_params);
	TEST_ASSERT_NULL(request.params);

	request_json = "{\"jsonrpc\": \"2.0\", \"method\": \"play\", \"params\" : {}}";

	status = Request_from_json(&request, request_json, strlen(request_json));

	TEST_ASSERT_EQUAL_INT(0, status);
	TEST_ASSERT_EQUAL_INT(0, request.num_params);
	TEST_ASSERT_NULL(request.params);
}

static void
test_deserialize_request()
{
	const char* request_json = "{\"jsonrpc\": \"2.0\", \"id\": 66, \"method\": \"play\", \"params\" : [\"hello world\"]}";

	RPC_Request request;
	int status = Request_from_json(&request, request_json, strlen(request_json));

	TEST_ASSERT_EQUAL_INT(0, status);
	TEST_ASSERT_EQUAL_INT(RPC_ERROR_OK, request.error);
	TEST_ASSERT_EQUAL_STRING("play", request.method_name);
	TEST_ASSERT_EQUAL_INT(RPC_ID_NUMBER, request.id_type);
	TEST_ASSERT_EQUAL_INT(66, request.id.number);
	TEST_ASSERT_EQUAL_INT(1, request.num_params);

	TEST_ASSERT_NULL(request.params[0].name);
	TEST_ASSERT_EQUAL_INT(0, request.params[0].position);
	TEST_ASSERT_EQUAL(RPC_TYPE_STRING, request.params[0].value.type);
	TEST_ASSERT_EQUAL_STRING("hello world", request.params[0].value.value.string);

	TEST_ASSERT_EQUAL_INT(request.error, RPC_ERROR_OK);

	if (request.f_free)
		request.f_free(&request);
}

int
main()
{
	TEST_BEGIN

	/* RPC parameter decoding in JSON format */
	RUN(test_json_integer_to_params);
	RUN(test_json_array_to_params);
	RUN(test_json_obj_param);

	/* RPC method calls with JSON encoded parameters */
	RUN(test_call_print_person_json);
	RUN(test_call_get_person_json);

	/* JSON RPC 2.0 request decoding tests */
	RUN(test_deserialize_error_request_parse);
	RUN(test_deserialize_error_request_invalid);
	RUN(test_deserialize_error_request_invalid__jsonrpc_type);
	RUN(test_deserialize_error_request_invalid__missing_method_param);
	RUN(test_deserialize_error_param_invalid_value__jsonrpc);
	RUN(test_deserialize_error_param_invalid_type__id);
	RUN(test_deserialize_request__id);
	RUN(test_deserialize_request__extraneous_attributes);
	RUN(test_deserialize_error_param_deserialize);
	RUN(test_deserialize_request_empty_params);
	RUN(test_deserialize_request);

	TEST_END
}
