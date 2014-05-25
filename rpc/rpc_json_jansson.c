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
		set_cx_errno(RPC_ERROR_INVALID_PARAMS);
		return -1;
	}
}

json_t*
Value_to_json(RPC_Value* value)
{
	if (!value)
	{
		XWARN("RPC_Value is null");
		return json_null();
	}

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
	case RPC_TYPE_VOID:
		return json_null();
	case RPC_TYPE_OBJECT:
		if (value->f_to_json)
			return value->f_to_json(value->value.object);
		else
			set_cx_errno(RPC_ERROR_METHOD_MISSING);
	}
	return NULL;
}

void
RPC_Request_json_free(RPC_Request* request)
{
	if (request->result.f_free)
		request->result.f_free(request->result.value.object);

	if (request->data)
		json_decref((json_t*)request->data);

	if (request->error_reason)
		cx_free(request->error_reason);

	if (request->params)
		cx_free(request->params);

	memset(request, 0, sizeof(RPC_Request));
}

static json_t*
request_id_to_json(RPC_Request* request)
{
	json_t* id = NULL;

	switch (request->id_type)
	{
	case RPC_ID_NONE:
		assert(false); // application error;
		break;
	case RPC_ID_NUMBER:
		id = json_integer(request->id.number);
		break;
	case RPC_ID_STRING:
		id = json_string(request->id.string);
		break;
	case RPC_ID_INVALID:
		id = json_null();
	}
	return id;
}

static json_t*
rpc_error_to_response(RPC_Request* request, JSON_RPC_Error json_rpc_error)
{
	json_t* id_json = request_id_to_json(request);

	// FIXME id must be set to request even if other validation failed !!!
	if (request->error_reason)
	{
		return json_pack("{s:s,s:o,s:{s:i,s:s,s:{s:s}}}",
				 "jsonrpc", "2.0", "id", id_json,
				 "error", "code", json_rpc_error, "message", "FIXME {implement strerror}",
				 "data", "reason", request->error_reason);
	}
	else
	{
		return json_pack("{s:s,s:o,s:{s:i,s:s}}",
				 "jsonrpc", "2.0", "id", id_json,
				 "error", "code", json_rpc_error, "message", "FIXME {implement strerror}");
	}
}

static json_t*
create_json_rpc_error(RPC_Request* request)
{
	switch (request->error)
	{
	case RPC_ERROR_REQUEST_PARSE:
		return rpc_error_to_response(request, JSON_RPC_ERROR_PARSE_ERROR);
	case RPC_ERROR_INVALID_REQUEST:
	case RPC_ERROR_INVALID_VERSION:
	case RPC_ERROR_INVALID_ID:
	case RPC_ERROR_INVALID_METHOD:
		return rpc_error_to_response(request, JSON_RPC_ERROR_INVALID_REQUEST);
	case RPC_ERROR_FORMAT_UNSUPPORTED:
		return rpc_error_to_response(request, JSON_RPC_ERROR_INTERNAL);
	case RPC_ERROR_METHOD_MISSING:
		return rpc_error_to_response(request, JSON_RPC_ERROR_METHOD_NOT_FOUND);
	case RPC_ERROR_NO_PARAMS:
	case RPC_ERROR_INVALID_PARAMS:
	case RPC_ERROR_PARAM_MISSING:
	case RPC_ERROR_PARAM_NULL:
	case RPC_ERROR_PARAM_INVALID_TYPE:
	case RPC_ERROR_PARAM_DESERIALIZE:
		return rpc_error_to_response(request, JSON_RPC_ERROR_INVALID_PARAMS);
	case RPC_ERROR_RESULT_VALUE_NULL:
	default:
		return rpc_error_to_response(request, JSON_RPC_ERROR_INTERNAL);
	}
	return NULL;
}

static json_t*
rpc_result_to_json(RPC_Request* request)
{
	json_t* id_json = request_id_to_json(request);
	json_t* result_json = NULL;

	/* can method methods only receive notifications ? */
	/* what should we return for void methods (result is required)  true, null, 0, {}, 42 ? */
	result_json = Value_to_json(&request->result);

	return json_pack("{s:s,s:o,s:o}",
			 "jsonrpc", "2.0", "id", id_json,
			 "result", result_json);
}

json_t*
Request_create_json_response(RPC_Request* request)
{
	json_t* response_json = NULL;

	switch (request->error)
	{
	case RPC_ERROR_OK:
		response_json = rpc_result_to_json(request);
		break;
	default:
		response_json = create_json_rpc_error(request);
		break;
	}

	if (!response_json)
	{
		XERR("Failed to create JSON RPC 2.0 response");
		// FIXME if json is null return static response string (internal error) ?
	}
	else
	{
		// FIXME only enable this if in debug mode
		json_dumpf(response_json, stderr, JSON_INDENT(2));
	}

	return response_json;
}

int
Request_json_parse(RPC_Request* request, const char* data, size_t data_len)
{
	json_error_t error;

	memset(&error, 0, sizeof(json_error_t));

	json_t* root = json_loadb(data, data_len, JSON_DECODE_ANY | JSON_REJECT_DUPLICATES, &error);

	if (!root)
	{
		memset(request, 0, sizeof(RPC_Request));
		request->f_free = RPC_Request_json_free;
		// FIXME use formatted message (using snprintf into the error buffer);
//		XFERR("JSON PARSE error: %s", error.text);
		RPC_Request_set_error(request, RPC_ERROR_REQUEST_PARSE, error.text);
		return -1;
	}
	else
	{
		if (Request_from_json(request, root) == -1)
		{
			return -1;
		}
	}
	return 0;
}

static int
deserialize_id(RPC_Request* request, json_t* id_json)
{
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
			RPC_Request_set_error(request, RPC_ERROR_INVALID_ID,
					      "parameter 'id': invalid format (expected integer or string)");
			request->id_type = RPC_ID_INVALID;
			return -1;
		}
	}
	return 0;
}

static int
deserialize_version(RPC_Request* request, json_t* jsonrpc_version_json)
{
	if (jsonrpc_version_json)
	{
		if (json_is_string(jsonrpc_version_json))
		{
			const char* jsonrpc_version = json_string_value(jsonrpc_version_json);

			if (strcmp(jsonrpc_version, JSONRPC_VERSION) != 0)
			{
				// FIXME use formatted message (using snprintf into the error buffer);
//				XFERR("Parameter 'jsonrpc' - invalid value [%s] (expected  '%s')", jsonrpc_version, JSONRPC_VERSION);
				RPC_Request_set_error(request, RPC_ERROR_INVALID_VERSION,
						      "Parameter 'jsonrpc' - invalid value (expected string \"2.0\")");
				return -1;
			}
		}
		else
		{
			RPC_Request_set_error(request, RPC_ERROR_INVALID_VERSION,
					      "Parameter 'jsonrpc' - invalid value (expected string \"2.0\")");
			return -1;
		}
	}
	else
	{
		RPC_Request_set_error(request, RPC_ERROR_INVALID_VERSION,
				      "Parameter 'jsonrpc' - not available");
		return -1;
	}
	return 0;
}

static int
deserialize_method_name(RPC_Request* request, json_t* method_name_json)
{
	if (method_name_json)
	{
		if (json_is_string(method_name_json))
		{
			const char* method_name = json_string_value(method_name_json);
			if (strlen(method_name) > 0)
			{
				request->method_name = method_name;
				return 0;
			}
			else
			{
				RPC_Request_set_error(request, RPC_ERROR_INVALID_METHOD,
						      "Parameter 'method' - is empty");
				return -1;
			}
		}
		else
		{
			RPC_Request_set_error(request, RPC_ERROR_INVALID_METHOD,
					      "Parameter 'method' - is not a string value");
			return -1;
		}
	}
	else
	{
		RPC_Request_set_error(request, RPC_ERROR_INVALID_METHOD,
				      "Parameter 'method' - is unavailable");
		return -1;
	}
}

static int
deserialize_params(RPC_Request* request, json_t* params_json)
{
	if (params_json)
	{
		int num_params = Params_from_json(&request->params, params_json);

		if (num_params == -1)
		{
			RPC_Request_set_error(request, (RPC_Error)cx_errno,
					      "failed to deserialize params");
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
	return 0;
}

int
Request_from_json(RPC_Request* request, json_t* request_json)
{
	/* clear request */
	memset(request, 0, sizeof(RPC_Request));

	request->format = FORMAT_JSON;
	request->f_free = RPC_Request_json_free;
	request->data = request_json;

	json_error_t error;
	memset(&error, 0, sizeof(json_error_t));

	json_t* method_name_json = NULL;
	json_t* jsonrpc_version_json = NULL;
	json_t* id_json = NULL;
	json_t* params_json = NULL;

	const char* format = "{s?:o, s?:o, s?:o, s?:o}";
	int unpack_status = json_unpack_ex(request_json, &error, 0, format,
					   "jsonrpc", &jsonrpc_version_json, "id", &id_json,
					   "method", &method_name_json, "params", &params_json);

	if (unpack_status != 0)         /* success */
	{
		XFERR("JSON unpack error: %s", error.text);
		// FIXME print to buffer with detailed information ?
		RPC_Request_set_error(request, RPC_ERROR_INVALID_REQUEST, error.text);
		return -1;
	}
	else
	{
		/* check {id} parameter first
		 * If other checks fail we have at least a valid id for the error
		 */
		if (deserialize_id(request, id_json) == -1)
			return -1;

		/* check {jsonrpc} version parameter */
		if (deserialize_version(request, jsonrpc_version_json) == -1)
			return -1;

		/* check {method} parameter */
		if (deserialize_method_name(request, method_name_json) == -1)
			return -1;

		/* check and deserialize {params} parameter */
		if (deserialize_params(request, params_json) == -1)
			return -1;
	}
	return 0;
}

/* FIXME convert to macro ? */
void
RPC_Request_set_error(RPC_Request* request, RPC_Error err, const char* error_reason)
{
	XFERR("Set error to request : [%d]{%s}", err, error_reason ? error_reason : "");
	request->error = err;
	if (error_reason)
	{
		request->error_reason = cx_strndup(error_reason, RPC_ERROR_MESSAGE_LENGTH_MAX);
	}
}
