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

static void
test_unpack()
{
//	const char* request = "{\"jsonrpc\": \"2.0\", \"id\": 66, \"method\": \"play\", \"params\" : [\"Nice\"]}";

//	const char* request = "{\"jsonrpc\": \"2.0\", \"id\": 66, \"method\": \"play\"}";
//	const char* request = "{\"id\": 66, \"method\": \"play\", \"jsonrpc\": \"2.0\"}";
	const char* request = "{\"jsonrpc\": \"2.0\", \"method\": 1234 }";
	json_error_t error;

	memset(&error, 0, sizeof(json_error_t));


	// deserialize JSON
	json_t* root = json_loads(request, JSON_DECODE_ANY, &error);

	if (!root)
	{
		XFDBG("JSON PARSE error: %s", error.text);
	}
	else
	{
		const char* version = NULL;
		const char* method = NULL;
		json_t* id = NULL;
		json_t* params = NULL;

		const char* format = "{s:s, s?:o, s:s, s?:o}";
		int status = json_unpack_ex(root, &error, JSON_DECODE_ANY, format,
					    "jsonrpc", &version,
					    "id", &id,
					    "method", &method,
					    "params", &params);

		if (status == 0) /* success */
		{
			/* check jsonrpc version */
			if (strcmp(version, "2.0") != 0)
			{
				XFERR("Invalid jsonrpc version[%s]", version);
				// FIXME error (return and free root);
			}

			/* check id format */
			if (!id)
			{
				// notification
				XDBG("is notification");
			}
			else
			{
				if (json_is_integer(id))
				{
					XFDBG("id:%lld", json_integer_value(id));
				}
				else if (json_is_string(id))
				{
					XFDBG("id:%s", json_string_value(id));
				}
				else
				{
					XERR("parameter 'id': invalid format (expected integer or string)");
					// FIXME error (return and free root);
				}
			}

			if (!params)
			{
				XDBG("no params given");
			}
			else
			{
				// params must be array or object
				if (json_is_array(params))
				{
					// extract by position
				}
				else if (json_is_object(id))
				{
					// extract by name
				}
				else
				{
					XERR("parameter 'params': invalid format (expected array or object)");
					// FIXME error (return and free root);
				}
			}

			XFDBG("version:%s method:%s", version, method);
		}
		else if (status == -1) /* failure */
		{
			XFERR("JSON PARSE error: %s", error.text);
		}
	}
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
	RUN(test_unpack);

	TEST_END
}
