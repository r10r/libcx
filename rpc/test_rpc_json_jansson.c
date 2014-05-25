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
	int status = Request_json_parse(&request, request_json, strlen(request_json));

	TEST_ASSERT_EQUAL_INT(-1, status);
	TEST_ASSERT_EQUAL_INT(RPC_ERROR_REQUEST_PARSE, request.error);

	TEST_ASSERT_NULL(request.method_name);
	TEST_ASSERT_EQUAL_INT(RPC_ID_INVALID, request.id_type);
	TEST_ASSERT_EQUAL_INT(0, request.id.number);
	TEST_ASSERT_EQUAL_INT(0, request.num_params);
	TEST_ASSERT_NULL(request.params);

	RPC_Request_json_free(&request);
}

static void
test_deserialize_error_request_invalid()
{
	const char* request_json = "{\"id\": 66, \"method\": \"play\"}";

	RPC_Request request;
	int status = Request_json_parse(&request, request_json, strlen(request_json));

	TEST_ASSERT_EQUAL_INT(-1, status);
	TEST_ASSERT_EQUAL_INT(RPC_ERROR_INVALID_VERSION, request.error);
	TEST_ASSERT_EQUAL_INT(66, request.id.number);
	TEST_ASSERT_EQUAL_STRING(NULL, request.method_name);
	TEST_ASSERT_EQUAL_INT(0, request.num_params);
	TEST_ASSERT_NULL(request.params);

	RPC_Request_json_free(&request);
}

static void
test_deserialize_error_request_invalid__jsonrpc_type()
{
	/* jsonrpc version value must be a string */
	const char* request_json = "{\"jsonrpc\": 2.0, \"id\": 66, \"method\": \"play\"}";

	RPC_Request request;
	int status = Request_json_parse(&request, request_json, strlen(request_json));

	TEST_ASSERT_EQUAL_INT(-1, status);
	TEST_ASSERT_EQUAL_INT(RPC_ERROR_INVALID_VERSION, request.error);
	TEST_ASSERT_EQUAL_INT(66, request.id.number);
	TEST_ASSERT_NULL(request.method_name);
	TEST_ASSERT_EQUAL_INT(0, request.num_params);
	TEST_ASSERT_NULL(request.params);

	RPC_Request_json_free(&request);
}

/* jsonrpc version value must be exactly '2.0' */
static void
test_deserialize_error_param_invalid_value__jsonrpc()
{
	const char* request_json = "{\"jsonrpc\": \"3.0\", \"id\": 66, \"method\": \"play\"}";

	RPC_Request request;
	int status = Request_json_parse(&request, request_json, strlen(request_json));

	TEST_ASSERT_EQUAL_INT(-1, status);
	TEST_ASSERT_EQUAL_INT(RPC_ERROR_INVALID_VERSION, request.error);
	TEST_ASSERT_EQUAL_INT(66, request.id.number);
	TEST_ASSERT_NULL(request.method_name);
	TEST_ASSERT_EQUAL_INT(0, request.num_params);
	TEST_ASSERT_NULL(request.params);

	RPC_Request_json_free(&request);
}

/* jsonrpc version value must be a string or an integer*/
static void
test_deserialize_error_param_invalid_type__id()
{
	const char* request_json;
	RPC_Request request;
	int status;

	request_json = "{\"jsonrpc\": \"2.0\", \"id\": null, \"method\": \"play\"}";
	status = Request_json_parse(&request, request_json, strlen(request_json));

	TEST_ASSERT_EQUAL_INT(-1, status);
	TEST_ASSERT_EQUAL_INT(RPC_ERROR_INVALID_ID, request.error);
	TEST_ASSERT_EQUAL_INT(0, request.id.number);
	TEST_ASSERT_EQUAL_INT(RPC_ID_INVALID, request.id_type);
	TEST_ASSERT_NULL(request.method_name);
	TEST_ASSERT_EQUAL_INT(0, request.num_params);
	TEST_ASSERT_NULL(request.params);

	RPC_Request_json_free(&request);


	request_json = "{\"jsonrpc\": \"2.0\", \"id\": true, \"method\": \"play\"}";
	status = Request_json_parse(&request, request_json, strlen(request_json));

	TEST_ASSERT_EQUAL_INT(-1, status);
	TEST_ASSERT_EQUAL_INT(RPC_ERROR_INVALID_ID, request.error);
	TEST_ASSERT_EQUAL_INT(0, request.id.number);
	TEST_ASSERT_EQUAL_INT(RPC_ID_INVALID, request.id_type);
	TEST_ASSERT_NULL(request.method_name);
	TEST_ASSERT_EQUAL_INT(0, request.num_params);
	TEST_ASSERT_NULL(request.params);

	RPC_Request_json_free(&request);


	request_json = "{\"jsonrpc\": \"2.0\", \"id\": {}, \"method\": \"play\"}";
	status = Request_json_parse(&request, request_json, strlen(request_json));

	TEST_ASSERT_EQUAL_INT(-1, status);
	TEST_ASSERT_EQUAL_INT(RPC_ERROR_INVALID_ID, request.error);
	TEST_ASSERT_EQUAL_INT(0, request.id.number);
	TEST_ASSERT_EQUAL_INT(RPC_ID_INVALID, request.id_type);
	TEST_ASSERT_NULL(request.method_name);
	TEST_ASSERT_EQUAL_INT(0, request.num_params);
	TEST_ASSERT_NULL(request.params);

	RPC_Request_json_free(&request);


	request_json = "{\"jsonrpc\": \"2.0\", \"id\": [\"66\"], \"method\": \"play\"}";
	status = Request_json_parse(&request, request_json, strlen(request_json));

	TEST_ASSERT_EQUAL_INT(-1, status);
	TEST_ASSERT_EQUAL_INT(RPC_ERROR_INVALID_ID, request.error);
	TEST_ASSERT_EQUAL_INT(0, request.id.number);
	TEST_ASSERT_EQUAL_INT(RPC_ID_INVALID, request.id_type);
	TEST_ASSERT_NULL(request.method_name);
	TEST_ASSERT_EQUAL_INT(0, request.num_params);
	TEST_ASSERT_NULL(request.params);

	RPC_Request_json_free(&request);
}

static void
test_deserialize_error_request_invalid__missing_method_param()
{
	/* jsonrpc version value must be a string */
	const char* request_json = "{\"jsonrpc\": \"2.0\", \"id\": 66}";

	RPC_Request request;
	int status = Request_json_parse(&request, request_json, strlen(request_json));

	TEST_ASSERT_EQUAL_INT(-1, status);
	TEST_ASSERT_EQUAL_INT(RPC_ERROR_INVALID_METHOD, request.error);
	TEST_ASSERT_EQUAL_INT(66, request.id.number);
	TEST_ASSERT_NULL(request.method_name);
	TEST_ASSERT_EQUAL_INT(0, request.num_params);
	TEST_ASSERT_NULL(request.params);

	RPC_Request_json_free(&request);
}

static void
test_deserialize_request__id()
{
	const char* request_json;
	RPC_Request request;
	int status;

	request_json = "{\"jsonrpc\": \"2.0\", \"id\": 66, \"method\": \"play\"}";
	status = Request_json_parse(&request, request_json, strlen(request_json));

	TEST_ASSERT_EQUAL_INT(0, status);
	TEST_ASSERT_EQUAL_INT(RPC_ERROR_OK, request.error);
	TEST_ASSERT_EQUAL_STRING("play", request.method_name);
	TEST_ASSERT_EQUAL_INT(RPC_ID_NUMBER, request.id_type);
	TEST_ASSERT_EQUAL_INT(66, request.id.number);
	TEST_ASSERT_EQUAL_INT(0, request.num_params);
	TEST_ASSERT_NULL(request.params);

	RPC_Request_json_free(&request);


	request_json = "{\"jsonrpc\": \"2.0\", \"id\": \"66\", \"method\": \"play\"}";
	status = Request_json_parse(&request, request_json, strlen(request_json));

	TEST_ASSERT_EQUAL_INT(0, status);
	TEST_ASSERT_EQUAL_INT(RPC_ERROR_OK, request.error);
	TEST_ASSERT_EQUAL_STRING("play", request.method_name);
	TEST_ASSERT_EQUAL_INT(RPC_ID_STRING, request.id_type);
	TEST_ASSERT_EQUAL_STRING("66", request.id.string);
	TEST_ASSERT_EQUAL_INT(0, request.num_params);
	TEST_ASSERT_NULL(request.params);

	RPC_Request_json_free(&request);


	request_json = "{\"jsonrpc\": \"2.0\", \"method\": \"play\"}";
	status = Request_json_parse(&request, request_json, strlen(request_json));

	TEST_ASSERT_EQUAL_INT(0, status);
	TEST_ASSERT_EQUAL_INT(RPC_ERROR_OK, request.error);
	TEST_ASSERT_EQUAL_STRING("play", request.method_name);
	TEST_ASSERT_EQUAL_INT(RPC_ID_NONE, request.id_type);
	TEST_ASSERT_NULL(request.id.string);
	TEST_ASSERT_EQUAL_INT(0, request.num_params);
	TEST_ASSERT_NULL(request.params);

	RPC_Request_json_free(&request);
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
	status = Request_json_parse(&request, request_json, strlen(request_json));

	TEST_ASSERT_EQUAL_INT(0, status);
	TEST_ASSERT_EQUAL_INT(RPC_ERROR_OK, request.error);
	TEST_ASSERT_EQUAL_STRING("play", request.method_name);
	TEST_ASSERT_EQUAL_INT(RPC_ID_NUMBER, request.id_type);
	TEST_ASSERT_EQUAL_INT(66, request.id.number);
	TEST_ASSERT_EQUAL_INT(0, request.num_params);
	TEST_ASSERT_NULL(request.params);

	RPC_Request_json_free(&request);
}

static void
test_deserialize_error_param_deserialize()
{
	const char* request_json;
	RPC_Request request;
	int status;

	request_json = "{\"jsonrpc\": \"2.0\", \"id\": \"foobar\", \"method\": \"play\", \"params\" : 12345 }";
	status = Request_json_parse(&request, request_json, strlen(request_json));

	TEST_ASSERT_EQUAL_INT(-1, status);
	TEST_ASSERT_EQUAL_INT(RPC_ERROR_INVALID_PARAMS, request.error);
	TEST_ASSERT_EQUAL_STRING("play", request.method_name);
	TEST_ASSERT_EQUAL_INT(RPC_ID_STRING, request.id_type);
	TEST_ASSERT_EQUAL_STRING("foobar", request.id.string);
	TEST_ASSERT_EQUAL_INT(0, request.num_params);
	TEST_ASSERT_NULL(request.params);

	RPC_Request_json_free(&request);


	request_json = "{\"jsonrpc\": \"2.0\", \"id\": \"foobar\", \"method\": \"play\", \"params\" : 2.0 }";
	status = Request_json_parse(&request, request_json, strlen(request_json));

	TEST_ASSERT_EQUAL_INT(-1, status);
	TEST_ASSERT_EQUAL_INT(RPC_ERROR_INVALID_PARAMS, request.error);
	TEST_ASSERT_EQUAL_STRING("play", request.method_name);
	TEST_ASSERT_EQUAL_INT(RPC_ID_STRING, request.id_type);
	TEST_ASSERT_EQUAL_STRING("foobar", request.id.string);
	TEST_ASSERT_EQUAL_INT(0, request.num_params);
	TEST_ASSERT_NULL(request.params);

	RPC_Request_json_free(&request);


	request_json = "{\"jsonrpc\": \"2.0\", \"id\": \"foobar\", \"method\": \"play\", \"params\" : \"foobar\" }";
	status = Request_json_parse(&request, request_json, strlen(request_json));

	TEST_ASSERT_EQUAL_INT(-1, status);
	TEST_ASSERT_EQUAL_INT(RPC_ERROR_INVALID_PARAMS, request.error);
	TEST_ASSERT_EQUAL_STRING("play", request.method_name);
	TEST_ASSERT_EQUAL_INT(RPC_ID_STRING, request.id_type);
	TEST_ASSERT_EQUAL_STRING("foobar", request.id.string);
	TEST_ASSERT_EQUAL_INT(0, request.num_params);
	TEST_ASSERT_NULL(request.params);

	RPC_Request_json_free(&request);


	request_json = "{\"jsonrpc\": \"2.0\", \"id\": \"foobar\", \"method\": \"play\", \"params\" : null }";
	status = Request_json_parse(&request, request_json, strlen(request_json));

	TEST_ASSERT_EQUAL_INT(-1, status);
	TEST_ASSERT_EQUAL_INT(RPC_ERROR_INVALID_PARAMS, request.error);
	TEST_ASSERT_EQUAL_STRING("play", request.method_name);
	TEST_ASSERT_EQUAL_INT(RPC_ID_STRING, request.id_type);
	TEST_ASSERT_EQUAL_STRING("foobar", request.id.string);
	TEST_ASSERT_EQUAL_INT(0, request.num_params);
	TEST_ASSERT_NULL(request.params);

	RPC_Request_json_free(&request);


	request_json = "{\"jsonrpc\": \"2.0\", \"id\": \"foobar\", \"method\": \"play\", \"params\" : true }";
	status = Request_json_parse(&request, request_json, strlen(request_json));

	TEST_ASSERT_EQUAL_INT(-1, status);
	TEST_ASSERT_EQUAL_INT(RPC_ERROR_INVALID_PARAMS, request.error);
	TEST_ASSERT_EQUAL_STRING("play", request.method_name);
	TEST_ASSERT_EQUAL_INT(RPC_ID_STRING, request.id_type);
	TEST_ASSERT_EQUAL_STRING("foobar", request.id.string);
	TEST_ASSERT_EQUAL_INT(0, request.num_params);
	TEST_ASSERT_NULL(request.params);

	RPC_Request_json_free(&request);
}

static void
test_deserialize_request_empty_params()
{
	const char* request_json;
	RPC_Request request;
	int status;

	request_json = "{\"jsonrpc\": \"2.0\",  \"id\": \"foobar\", \"method\": \"play\", \"params\" : []}";

	status = Request_json_parse(&request, request_json, strlen(request_json));

	TEST_ASSERT_EQUAL_INT(0, status);
	TEST_ASSERT_EQUAL_INT(RPC_ID_STRING, request.id_type);
	TEST_ASSERT_EQUAL_STRING("foobar", request.id.string);
	TEST_ASSERT_EQUAL_INT(0, request.num_params);
	TEST_ASSERT_NULL(request.params);

	RPC_Request_json_free(&request);


	request_json = "{\"jsonrpc\": \"2.0\",  \"id\": \"foobar\", \"method\": \"play\", \"params\" : {}}";
	status = Request_json_parse(&request, request_json, strlen(request_json));

	TEST_ASSERT_EQUAL_INT(0, status);
	TEST_ASSERT_EQUAL_INT(RPC_ID_STRING, request.id_type);
	TEST_ASSERT_EQUAL_STRING("foobar", request.id.string);
	TEST_ASSERT_EQUAL_INT(0, request.num_params);
	TEST_ASSERT_NULL(request.params);

	RPC_Request_json_free(&request);
}

static void
test_deserialize_request()
{
	const char* request_json = "{\"jsonrpc\": \"2.0\", \"id\": 66, \"method\": \"play\", \"params\" : [\"hello world\"]}";

	RPC_Request request;
	int status = Request_json_parse(&request, request_json, strlen(request_json));

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

	RPC_Request_json_free(&request);
}

static void
test_Request_create_json_response_error()
{
	RPC_Request request = {
		.id_type = RPC_ID_INVALID
	};

	RPC_Request_set_error(&request, RPC_ERROR_REQUEST_PARSE, "My error reason");

	json_t* response = Request_create_json_response(&request);

	const char* jsonrpc_version = NULL;
	json_t* id_json = NULL;
	int error_code;
	const char* error_message = NULL;
	const char* error_reason = NULL;

	json_error_t error;
	memset(&error, 0, sizeof(json_error_t));

	int status = json_unpack_ex(response, &error, 0, "{s:s,s?:o,s:{s:i,s:s,s?:{s:s}}}",
				    "jsonrpc", &jsonrpc_version,
				    "id", &id_json,
				    "error", "code", &error_code, "message", &error_message,
				    "data", "reason", &error_reason);

	TEST_ASSERT_EQUAL_INT(0, status);
	TEST_ASSERT_EQUAL_STRING(JSONRPC_VERSION, jsonrpc_version);
	TEST_ASSERT_EQUAL_INT(JSON_RPC_ERROR_PARSE_ERROR, error_code);
	TEST_ASSERT_EQUAL_STRING("Parse error", error_message);
	TEST_ASSERT_EQUAL_STRING("My error reason", error_reason);
	TEST_ASSERT_TRUE(json_is_null(id_json));

	RPC_Request_json_free(&request);
}

static void
test_Request_create_json_response_error__no_reason()
{
	;
	RPC_Request request = {
		.id_type = RPC_ID_STRING,
		.id.string = "myid"
	};

	RPC_Request_set_error(&request, RPC_ERROR_REQUEST_PARSE, NULL);
	json_t* response = Request_create_json_response(&request);

	const char* jsonrpc_version = NULL;
	json_t* id_json = NULL;
	int error_code;
	const char* error_message = NULL;
	const char* error_reason = NULL;

	json_error_t error;
	memset(&error, 0, sizeof(json_error_t));

	int status = json_unpack_ex(response, &error, 0, "{s:s,s?:o,s:{s:i,s:s,s?:{s:s}}}",
				    "jsonrpc", &jsonrpc_version,
				    "id", &id_json,
				    "error", "code", &error_code, "message", &error_message,
				    "data", "reason", &error_reason);

	TEST_ASSERT_EQUAL_INT(0, status);
	TEST_ASSERT_EQUAL_STRING(JSONRPC_VERSION, jsonrpc_version);
	TEST_ASSERT_EQUAL_INT(JSON_RPC_ERROR_PARSE_ERROR, error_code);
	TEST_ASSERT_EQUAL_STRING("Parse error", error_message);
	TEST_ASSERT_NULL(error_reason);
	TEST_ASSERT_EQUAL_STRING("myid", json_string_value(id_json));

	RPC_Request_json_free(&request);
	json_decref(response);
}

// TODO test setting the ID

static void
test_Request_create_response()
{
	Person person = {
		.firstname      = "Max",
		.lastname       = "Mustermann",
		.age            = 33
	};

	RPC_Request request = {
		.method_name = "print_person",
		.error = RPC_ERROR_OK,
		.id_type = RPC_ID_STRING,
		.id.string = "myid",
		.result = {
			.type = RPC_TYPE_OBJECT,
			.value.object = &person,
			.f_to_json = (F_ValueToJSON*)&Person_to_json
		}
	};

	json_t* response = Request_create_json_response(&request);

	TEST_ASSERT_NOT_NULL(response);
	json_dumpf(response, stdout, JSON_INDENT(2));

	RPC_Request_json_free(&request);
	json_decref(response);
}

int
main()
{
	TEST_BEGIN

	// FIXME rename test methods to test_Request_json_parse_*

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

	/* JSON RPC 2.0 method calls */
	RUN(test_Request_create_json_response_error);
	RUN(test_Request_create_json_response_error__no_reason);
	RUN(test_Request_create_response);

	TEST_END
}
