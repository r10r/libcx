#include "rpc_json.h"
#include <jansson.h>

static void
deserialize_param_int(RPC_Param* param, json_t* json)
{
	long long long_val = json_integer_value(json);

	if (long_val <= INT_MAX && long_val >= INT_MIN)
	{
//		XFDBG("[%d](%s): (int)%d", index, key, (int)long_val);
		param->value.type = RPC_TYPE_INTEGER;
		param->value.value.integer = (int)long_val;
	}
	else
	{
//		XFDBG("[%d](%s): (long long)%lld", index, key, long_val);
		param->value.type = RPC_TYPE_LONGLONG;
		param->value.value.longlong = long_val;
	}
}

static void
deserialize_param(RPC_Param* param, int position, const char* key, json_t* json)
{
	param->position = position;
	param->name = key;

	if (json_is_null(json))
	{
		param->value.type = RPC_TYPE_NULL;
	}
	else if (json_is_integer(json))
	{
		deserialize_param_int(param, json);
	}
	else if (json_is_boolean(json))
	{
		param->value.type = RPC_TYPE_BOOLEAN;
		param->value.value.boolean = json_is_true(json) ? true : false;
	}
	else if (json_is_real(json))
	{
		param->value.type = RPC_TYPE_DOUBLE;
		param->value.value.floatingpoint = json_real_value(json);
	}
	else if (json_is_string(json))
	{
		param->value.type = RPC_TYPE_STRING;
		param->value.value.string = json_string_value(json);
	}
	else if (json_is_array(json) || json_is_object(json))
	{
		param->value.type = RPC_TYPE_OBJECT;
		param->value.value.object = json;
	}
}

static int
params_from_object(RPC_Param** params, json_t* json)
{
	size_t num_params = json_object_size(json);

	if (num_params > 0)
	{
		*params = cx_alloc(num_params * sizeof(RPC_Param));
		void* iter = json_object_iter(json);
		RPC_Param* param = params[0];
		size_t index = 0;
		while (iter != NULL)
		{
			const char* key = json_object_iter_key(iter);
			json_t* value = json_object_iter_value(iter);

			deserialize_param(param, -1, key, value);

			iter = json_object_iter_next(json, iter);
			param++;
			index++;
		}
	}
	return (int)num_params;
}

static int
params_from_array(RPC_Param** params, json_t* json)
{
	size_t num_params = json_array_size(json);

	if (num_params > 0)
	{
		*params = cx_alloc(num_params * sizeof(RPC_Param));

		RPC_Param* param = params[0];
		json_t* value = NULL;
		size_t index;
		json_array_foreach(json, index, value)
		{
			deserialize_param(param, (int)index, NULL, value);
			param++;
		}
	}

	return (int)num_params;
}

int
Params_from_json(RPC_Param** params, json_t* json)
{
	if (json_is_object(json))
	{
		return params_from_object(params, json);
	}
	else if (json_is_array(json))
	{
		return params_from_array(params, json);
	}
	else
	{
		XERR("parameter 'params': invalid format (expected array or object)");
		set_cx_errno(RPC_ERROR_PARAM_INVALID_TYPE);
		return -1;
	}
}

json_t*
Value_to_json(RPC_Value* value)
{
	switch (value->type)
	{
	case RPC_TYPE_INTEGER:
		return json_integer(value->value.integer);
	case RPC_TYPE_LONGLONG:
		return json_integer(value->value.longlong);
	case RPC_TYPE_DOUBLE:
		return json_real(value->value.floatingpoint);
	case RPC_TYPE_STRING:
		return json_string(value->value.string);
	case RPC_TYPE_BOOLEAN:
		return json_boolean(value->value.boolean);
	case RPC_TYPE_NULL:
		return json_null();
	case RPC_TYPE_OBJECT:
		if (value->f_to_json)
			return value->f_to_json(value->value.object);
		else
			set_cx_errno(RPC_ERROR_METHOD_MISSING);
	}
	return NULL;
}

#define JSONRPC_VERSION "2.0"

static void
RPC_Request_json_free(RPC_Request* request)
{
	if (request->data)
		json_decref((json_t*)request->data);
	if (request->params)
		cx_free(request->params);
}

int
Request_from_json(RPC_Request* request, const char* data, size_t data_len)
{
	/* clear request */
	memset(request, 0, sizeof(RPC_Request));

	json_error_t error;

	memset(&error, 0, sizeof(json_error_t));

	json_t* root = json_loadb(data, data_len, JSON_DECODE_ANY | JSON_REJECT_DUPLICATES, &error);

	if (!root)
	{
		XFERR("JSON PARSE error: %s", error.text);
		memset(request, 0, sizeof(RPC_Request));
		request->error = RPC_ERROR_REQUEST_PARSE;
		return -1;
	}
	else
	{
		const char* jsonrpc_version = NULL;
		const char* method_name = NULL;
		json_t* id_json = NULL;
		json_t* params_json = NULL;

		const char* format = "{s:s, s?:o, s:s, s?:o}";
		int unpack_status = json_unpack_ex(root, &error, 0, format,
						   "jsonrpc", &jsonrpc_version, "id", &id_json,
						   "method", &method_name, "params", &params_json);

		if (unpack_status != 0) /* success */
		{
			XFERR("JSON unpack error: %s", error.text);
			memset(request, 0, sizeof(RPC_Request));
			request->error = RPC_ERROR_REQUEST_INVALID;
			json_decref(root);
			return -1;
		}
		else
		{
			/* check jsonrpc version */
			if (strcmp(jsonrpc_version, "2.0") != 0)
			{
				XFERR("Parameter 'jsonrpc' - invalid value [%s] (expected  '%s')", jsonrpc_version, JSONRPC_VERSION);
				memset(request, 0, sizeof(RPC_Request));
				request->error = RPC_ERROR_PARAM_INVALID_VALUE;
				json_decref(root);
				return -1;
			}

			if (strlen(method_name) > 0)
			{
				request->method_name = method_name;
			}
			else
			{
				XERR("Parameter 'method' - is empty");
				memset(request, 0, sizeof(RPC_Request));
				request->error = RPC_ERROR_PARAM_INVALID_VALUE;
				json_decref(root);
				return -1;
			}

			/* check id format */
			if (!id_json)
			{
				/* notification */
				request->id_type = RPC_ID_NONE;
			}
			else
			{
				if (json_is_integer(id_json))
				{
					request->id_type = RPC_ID_NUMBER;
					request->id.number = json_integer_value(id_json);
				}
				else if (json_is_string(id_json))
				{
					request->id_type = RPC_ID_STRING;
					request->id.string = json_string_value(id_json);
				}
				else
				{
					XERR("parameter 'id': invalid format (expected integer or string)");
					memset(request, 0, sizeof(RPC_Request));
					request->error = RPC_ERROR_PARAM_INVALID_TYPE;
					json_decref(root);
					return -1;
				}
			}

			if (params_json)
			{
				int num_params = Params_from_json(&request->params, params_json);

				if (num_params == -1)
				{
					XERR("Failed to set request parameters");
					memset(request, 0, sizeof(RPC_Request));
					request->error = (RPC_Error)cx_errno;
					json_decref(root);
					return -1;
				}
				else
				{
					request->num_params = num_params;
				}
			}
			else
			{
				XDBG("No params");
			}
		}
	}

	request->format = FORMAT_JSON;
	request->data = root;
	request->f_free = RPC_Request_json_free;
	return 0;
}
