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
		param->value.data.integer = (int)long_val;
	}
	else
	{
//		XFDBG("[%d](%s): (long long)%lld", index, key, long_val);
		param->value.type = RPC_TYPE_LONGLONG;
		param->value.data.longlong = long_val;
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
		param->value.data.boolean = json_is_true(json) ? true : false;
	}
	else if (json_is_real(json))
	{
		param->value.type = RPC_TYPE_DOUBLE;
		param->value.data.floatingpoint = json_real_value(json);
	}
	else if (json_is_string(json))
	{
		param->value.type = RPC_TYPE_STRING;
		param->value.data.string = json_string_value(json);
	}
	else if (json_is_array(json) || json_is_object(json))
	{
		param->value.type = RPC_TYPE_OBJECT;
		param->value.data.object = json;
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
		cx_err_set(CX_RPC_ERROR_INVALID_PARAMS, "parameter 'params': invalid format (expected array or object)");
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
		return json_integer(value->data.integer);
	case RPC_TYPE_LONGLONG:
		return json_integer(value->data.longlong);
	case RPC_TYPE_DOUBLE:
		return json_real(value->data.floatingpoint);
	case RPC_TYPE_STRING:
		return json_string(value->data.string);
	case RPC_TYPE_BOOLEAN:
		return json_boolean(value->data.boolean);
	case RPC_TYPE_NULL:
	case RPC_TYPE_VOID:
		return json_null();
	case RPC_TYPE_OBJECT:
		if (value->f_to_json)
			return value->f_to_json(value->data.object);
		else
			cx_err_set(CX_RPC_ERROR_INTERNAL, "Failed to convert value to JSON");
	}
	return NULL;
}

static json_t*
request_id_to_json(RPC_Request* request)
{
	json_t* id = NULL;

	switch (request->id_type)
	{
	case RPC_ID_NUMBER:
		id = json_integer(request->id.number);
		break;
	case RPC_ID_STRING:
		id = json_string(request->id.string);
		break;
	case RPC_ID_NONE:                       /* ID can be none here if json was parsed properly but request is still invalid */
	case RPC_ID_INVALID:
		id = json_null();
	}
	return id;
}

static json_t*
create_json_rpc_response_error(RPC_Request* request)
{
	json_t* id_json = request_id_to_json(request);

	json_t* json = NULL;
	json_error_t error_pack;

	memset(&error_pack, 0, sizeof(json_error_t));

	assert(id_json); /* application bug */

	// FIXME id must be set to request even if other validation failed !!!
	if (request->result.value.type != RPC_TYPE_VOID)
	{
		json = json_pack_ex(&error_pack, 0, "{s:s,s:o,s:{s:i,s:s,s:{s,o,s:s}}}",
				    "jsonrpc", "2.0", "id", id_json,
				    "error", "code", request->result.error, "message", cx_rpc_strerror(request->result.error),
				    "data", "details", Value_to_json((RPC_Value*)&request->result), "token", request->request->id);
	}
	else
	{
		json = json_pack_ex(&error_pack, 0, "{s:s,s:o,s:{s:i,s:s,s:{s:s}}}",
				    "jsonrpc", "2.0", "id", id_json,
				    "error", "code", request->result.error, "message", cx_rpc_strerror(request->result.error),
				    "data", "token", request->request->id);
	}

	if (!json)
	{
		XFERR("Error creating error response (request %p): %s", (void*)request, error_pack.text);
		assert(false);  /* application bug */
	}

	return json;
}

static json_t*
RPC_Request_create_error(Request* request, RPC_Error err, const char* message)
{
	json_t* response_json;
	RPC_Request rpc_request;

	RPC_Request_init(&rpc_request, request);
	RPC_Result_set_error(&rpc_request.result, err, message);
	response_json = create_json_rpc_response_error(&rpc_request);
	RPC_Request_free(&rpc_request);
	return response_json;
}

static json_t*
create_json_rpc_response(RPC_Request* request)
{
	json_t* id_json = request_id_to_json(request);
	json_t* result_json = NULL;

	/* can method methods only receive notifications ? */
	/* what should we return for void methods (result is required)  true, null, 0, {}, 42 ? */
	result_json = Value_to_json((RPC_Value*)&request->result);

	return json_pack("{s:s,s:o,s:o}",
			 "jsonrpc", "2.0", "id", id_json,
			 "result", result_json);
}

json_t*
Request_create_json_response(RPC_Request* request)
{
	json_t* response_json = NULL;

	if (request->result.error == CX_ERR_OK)
		response_json = create_json_rpc_response(request);
	else
		response_json = create_json_rpc_response_error(request);

	if (!response_json)
	{
		/* could happen because of an internal error in jansson
		 * (e.g memory allocation failed) ...
		 */
		XERR("Failed to create JSON RPC 2.0 response");
	}

	return response_json;
}

static json_t*
process_request(RPC_MethodTable* rpc_methods, Request* request, json_t* request_json)
{
	RPC_Request rpc_request;

	RPC_Request_init(&rpc_request, request);

	int status = Request_from_json(&rpc_request, request_json);
	if (status == 0)
	{
		Service_call(rpc_methods, &rpc_request);
		XFLOG("RPC method(%s) executed (with return value)", rpc_request.method_name);
	}
	json_t* response_json = NULL;

	/* check whether the it is a notification */
	if (rpc_request.result.error == CX_RPC_ERROR_INVALID_REQUEST
	    || rpc_request.id_type != RPC_ID_NONE)
	{
		response_json = Request_create_json_response(&rpc_request);
#ifdef _CX_DEBUG
		XFDBG("Generated response for request[%s]", request->id);
		fprintf(stderr, "<----------\n");
		json_dumpf(response_json, stderr, JSON_INDENT(2));
		fprintf(stderr, "\n<----------\n");
#endif
	}
	else
	{
		XDBG("Ignoring result from method call. Request is a notification\n");
	}

	RPC_Request_free(&rpc_request);
	return response_json;
}

static json_t*
process_batch_request(RPC_MethodTable* rpc_methods, Request* request, json_t* batch_request_json)
{
	/* batch request */
	size_t num_requests = json_array_size(batch_request_json);

	XFDBG("Batch request[%s] has %zu elements", request->id, num_requests);
	if (json_array_size(batch_request_json) > 0)
	{
		json_t* batch_response_json = json_array();

		size_t index;
		json_t* request_json = NULL;
		json_array_foreach(batch_request_json, index, request_json)
		{
			json_t* response_json = process_request(rpc_methods, request, request_json);

			if (response_json)
				json_array_append_new(batch_response_json, response_json);
		}

		/* do not return an empty batch response */
		if (json_array_size(batch_response_json) == 0)
		{
			json_decref(batch_response_json);
			return NULL;
		}
		else
			return batch_response_json;
	}
	else
	{
		return RPC_Request_create_error(request, CX_RPC_ERROR_INVALID_REQUEST, "Empty batch request");
	}
}

json_t*
RPC_process(RPC_MethodTable* rpc_methods, Request* request)
{
	/* initialize error */
	json_error_t error;

	memset(&error, 0, sizeof(json_error_t));

	char* payload = NULL;
	size_t payload_len = request->f_get_payload(request, &payload);
	json_t* root_json = json_loadb(payload, payload_len, JSON_DECODE_ANY | JSON_REJECT_DUPLICATES, &error);

	json_t* response_json = NULL;
	if (root_json)
	{
		#ifdef _CX_DEBUG
		XFDBG("Processing request[%s]", request->id);
		fprintf(stderr, "---------->\n");
		json_dumpf(root_json, stderr, JSON_INDENT(2));
		fprintf(stderr, "\n---------->\n");
		#endif


		/* valid JSON */
		if (json_is_object(root_json))
		{
			/* single rpc */
			response_json = process_request(rpc_methods, request, root_json);
		}
		else if (json_is_array(root_json))
		{
			/* batch rpc */
			response_json = process_batch_request(rpc_methods, request, root_json);
		}
		else
		{
			response_json = RPC_Request_create_error(request, CX_RPC_ERROR_INVALID_REQUEST, "Invalid request value (expected array or object)");
		}
		json_decref(root_json);
	}
	else
	{
		response_json = RPC_Request_create_error(request, CX_RPC_ERROR_PARSE, error.text);
	}
	return response_json;
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
			RPC_Result_set_error(&request->result, CX_RPC_ERROR_INVALID_REQUEST,
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
				RPC_Result_set_error(&request->result, CX_RPC_ERROR_INVALID_REQUEST,
						     "Parameter 'jsonrpc' - invalid value (expected string \"2.0\")");
				return -1;
			}
		}
		else
		{
			RPC_Result_set_error(&request->result, CX_RPC_ERROR_INVALID_REQUEST,
					     "Parameter 'jsonrpc' - invalid value (expected string \"2.0\")");
			return -1;
		}
	}
	else
	{
		RPC_Result_set_error(&request->result, CX_RPC_ERROR_INVALID_REQUEST,
				     "Parameter 'jsonrpc' - is unavailable");
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
				if (strncasecmp(JSONRPC_RESERVED_METHOD_PREFIX, method_name,
						JSONRPC_RESERVED_METHOD_PREFIX_LEN) == 0)
				{
					RPC_Result_set_error(&request->result, CX_RPC_ERROR_INVALID_REQUEST,
							     "Parameter 'method' begins with the reserved prefix 'rpc.' (internal use only).");
					return -1;
				}
				else
				{
					request->method_name = method_name;
					return 0;
				}
			}
			else
			{
				RPC_Result_set_error(&request->result, CX_RPC_ERROR_INVALID_REQUEST,
						     "Empty 'method' parameter");
				return -1;
			}
		}
		else
		{
			RPC_Result_set_error(&request->result, CX_RPC_ERROR_INVALID_REQUEST,
					     "Parameter 'method' - is not a string value");
			return -1;
		}
	}
	else
	{
		RPC_Result_set_error(&request->result, CX_RPC_ERROR_INVALID_REQUEST,
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
			RPC_Result_set_error(&request->result, cx_err_code, cx_rpc_strerror(cx_err_code));
			return -1;
		}
		else
			request->num_params = num_params;
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
	request->format = FORMAT_JSON;

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
		RPC_Result_set_error(&request->result, CX_RPC_ERROR_INVALID_REQUEST, error.text);
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
