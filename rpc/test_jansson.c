#include "base/test.h"
#include <jansson.h>
#include "string/string_buffer.h"

static void
test_manual_building()
{
	json_t* json;

//	json_error_t error;

//	json->type;
//	json_typeof(json);

	json = json_array();

	json_t* value1 = json_string("foobar");
	json_t* value2 = json_string("helloworld");

	json_array_append(json, value1);
	json_array_append(json, value2);

	size_t flags = JSON_INDENT(2);
	char* data = json_dumps(json, flags);

	printf("%s\n", data);

	/* each created value must be dereferenced */
	json_decref(value1);
	json_decref(value2);
	json_decref(json);

	free(data);
}

static void
test_pack()
{
	json_t* json = json_pack("[s,s]", "foobar", "helloworld");
	size_t flags = JSON_INDENT(2);
	char* data = json_dumps(json, flags);

	printf("%s\n", data);

	/* using pack only the main object must be dereferenced */
	json_decref(json);

	free(data);
}

static void
test_pack_manual()
{
	json_t* json;
//	json_error_t error;

//	json->type;
//	json_typeof(json);

	json_t* value1 = json_string("foobar");
	json_t* value2 = json_string("helloworld");

	json = json_pack("[o,o]", value1, value2);


	size_t flags = JSON_INDENT(2);
	char* data = json_dumps(json, flags);
	printf("%s\n", data);

	/* each created value must be dereferenced */
	json_decref(json);

	free(data);
}

static void
test_pack_nested()
{
	json_t* json = json_object();

	/* use _set_new instead of _set to not increment the reference count */
	json_object_set_new(json, "foo", json_string("bar"));
	json_object_set_new(json, "hello", json_string("world"));

	size_t flags = JSON_INDENT(2);
	char* data = json_dumps(json, flags);
	printf("%s\n", data);

	/* each created value must be dereferenced */
	json_decref(json);

	free(data);
}

static int
dump_callback(const char* buffer, size_t size, void* data)
{
	XFDBG("buff[%s] size:%lu", buffer, size);
	StringBuffer_ncat((StringBuffer*)data, buffer, size);
	return 0; /* 0 succcess, -1 to stop encoding */
}

static void
test_dump_callback()
{
	json_t* json = json_object();

	/* use _set_new instead of _set to not increment the reference count */
	json_object_set_new(json, "foo", json_string("bar"));
	json_object_set_new(json, "hello", json_string("world"));

	StringBuffer* buff = StringBuffer_new(1024);
	json_dump_callback(json, dump_callback, buff, 0);

	XFDBG("%s", StringBuffer_value(buff));

	char* s =  json_dumps(json, 0);
	TEST_ASSERT_EQUAL_STRING(s, StringBuffer_value(buff));
	cx_free(s);

	StringBuffer_free(buff);
	json_decref(json);
}

static void
test_create_jsonrpc_request()
{
	// create JSONRPC 2.0 request
	json_t* json = json_object();

	json_object_set_new(json, "jsonrpc", json_string("2.0"));
	json_object_set_new(json, "id", json_integer(1));
	json_object_set_new(json, "method", json_string("echo"));

	json_t* params = json_object();
	json_object_set_new(params, "input", json_string("hello world"));
	json_object_set_new(json, "params", params);

	size_t flags = JSON_INDENT(2);

	StringBuffer* buff = StringBuffer_new(128);
	json_dump_callback(json, dump_callback, buff, flags);
	printf("%s\n", StringBuffer_value(buff));

	StringBuffer_free(buff);
	json_decref(json);
}

int
main()
{
	TEST_BEGIN
	RUN(test_manual_building);
	RUN(test_pack);
	RUN(test_pack_manual);
	RUN(test_pack_nested);
	RUN(test_dump_callback);
	RUN(test_create_jsonrpc_request);

	TEST_END
}
