#include "jsrpc.h"
#include "list/list.h"
#include "yajl/yajl_tree.h"

static const char* JSONRPC_VERSION_PATH[] = { "jsonrpc", NULL };
static const char* JSONRPC_ID_PATH[] = { "id", NULL };
static const char* JSONRPC_METHOD_PATH[] = { "method", NULL };
static const char* JSONRPC_PARAMS_PATH[] = { "params", NULL };

static yajl_val
get_param_value(RPC_Request* request, RPC_Param* param)
{
	yajl_val params = yajl_tree_get((yajl_val)request->data, JSONRPC_PARAMS_PATH, yajl_t_any);

	if (YAJL_IS_ARRAY(params))
	{
		// check bounds
		if (param->pos < (int)params->u.array.len)
			return *(params->u.array.values + param->pos);
		else
			XFERR("Param index[%d] out of bounds", param->pos);
	}
	else if (YAJL_IS_OBJECT(params))
	{
		const char* parameter_path[] = { param->name, NULL };
		return yajl_tree_get(params, parameter_path, yajl_t_any);
	}

	request->error = jsrpc_ERROR_INVALID_PARAMS;
	return NULL;
}

const char*
RPC_Request_get_param_value_string(RPC_Request* request, RPC_Param* param)
{
	yajl_val value = get_param_value(request, param);

	if (value)
	{
		switch (value->type)
		{
		case yajl_t_null:
			break;
		case yajl_t_string:
			XFDBG("Deserialized parameter[%d] %s --> %s",
			      param->pos, param->name, value->u.string);
			return value->u.string;
		default:
		{
			XFERR("Parameter [%d:%s] is not a string value.",
			      param->pos, param->name);
			request->error = jsrpc_ERROR_INVALID_PARAMS;
			break;
		}
		}
	}
	else
		XFERR("Missing param[%d] named '%s'", param->pos, param->name);

	return NULL;
}

long long
RPC_Request_get_param_value_longlong(RPC_Request* request, RPC_Param* param)
{
	yajl_val value = get_param_value(request, param);

	if (value)
	{
		if (YAJL_IS_INTEGER(value))
			return YAJL_GET_INTEGER(value);
		else
			XFERR("Parameter [%d:%s] is not a longlong value.",
			      param->pos, param->name);
	}
	else
		XFERR("Missing param[%d] named '%s'",
		      param->pos, param->name);

	return 0;
}

int
RPC_Request_get_param_value_int(RPC_Request* request, RPC_Param* param)
{
	long long value = RPC_Request_get_param_value_longlong(request, param);

	XFCHECK((value <= INT_MAX), "ERROR: Param %s is larger than INT_MAX", param->name);
	return (int)value;
}

double
RPC_Request_get_param_value_double(RPC_Request* request, RPC_Param* param)
{
	yajl_val value = get_param_value(request, param);

	if (value)
	{
		if (YAJL_IS_DOUBLE(value))
			return YAJL_GET_DOUBLE(value);
		else
			XFERR("Parameter [%d:%s] is not a double value.",
			      param->pos, param->name);
	}
	else
		XFERR("Missing param[%d] named '%s'", param->pos, param->name);

	return 0;
}

static bool
check_jsonrpc_version(yajl_val root)
{
	yajl_val v = yajl_tree_get(root, JSONRPC_VERSION_PATH, yajl_t_any);

	if (v)
	{
		char* jsonrpc_version = YAJL_GET_STRING(v);

		if (jsonrpc_version)
		{
			if (strcmp(jsonrpc_version, "2.0") == 0)
				return true;
			else
				XFERR("Unsupported jsonrpc version '%s'", jsonrpc_version);
		}
		else
			XFERR("Invalid type %d for property 'jsonrpc'", v->type);
	}
	else
		XERR("No 'jsonrpc' version string");

	return false;
}

/* @return false if request is malformed, true else */
static bool
set_request_id(RPC_Request* request, yajl_val root)
{
	yajl_val v = yajl_tree_get(root, JSONRPC_ID_PATH, yajl_t_any);

	if (v)
	{
		if (YAJL_IS_INTEGER(v))
		{
			request->id = YAJL_GET_NUMBER(v);
			request->id_numerical = true;
			return true;
		}
		else if (YAJL_IS_STRING(v))
		{
			request->id = YAJL_GET_STRING(v);
			request->id_numerical = false;
			return true;
		}
		else if (YAJL_IS_NULL(v))
		{
			XERR("Using null as id is discouraged! Treated as invalid request");
			return false;
		}
		else
		{
			XERR("Invalid value for property id");
			return false;
		}
	}
	else
	{
		XDBG("Received a notification");
		request->notification = true;
		return true;
	}
}

/* @return 0 if request is malformed, 1 else */
static bool
set_request_method(RPC_Request* request, yajl_val root)
{
	yajl_val v = yajl_tree_get(root, JSONRPC_METHOD_PATH, yajl_t_any);

	if (v)
	{
		request->method_name = YAJL_GET_STRING(v);
		if (request->method_name)
			return true;
		else
			XFDBG("Invalid type %d for parameter 'method'", v->type);
	}
	else
		XDBG("Request has no parameter 'method'");
	return false;
}

/* @return 0 if request is malformed, 1 else */
static bool
RPC_Request_parse(RPC_Request* request)
{
	bool success;
	yajl_val root = (yajl_val)request->data;

	success = check_jsonrpc_version(root);
	if (!success)
		return false;

	success = set_request_id(request, root);
	if (!success)
		return false;

	success = set_request_method(request, root);
	if (!success)
		return false;

	return true;
}

int
RPC_Request_deserialize(RPC_RequestList* request_list)
{
	char errbuf[1024];

	request_list->nrequests = 0;

	yajl_val json_root = yajl_tree_parse(StringBuffer_value(request_list->request_buffer), errbuf, sizeof(errbuf));

	request_list->data = json_root;

	if (json_root)
	{
		switch (json_root->type)
		{
		case yajl_t_array:
		{
			request_list->nrequests = (int)json_root->u.array.len;
			request_list->batch = 1;

			if (request_list->nrequests > 0)
			{
				request_list->requests = cx_alloc((size_t)request_list->nrequests * sizeof(RPC_Request));
				int i;
				for (i = 0; i < request_list->nrequests; i++)
				{
					RPC_Request* request = request_list->requests + i;
					request->data = json_root->u.array.values[i];
					if (!RPC_Request_parse(request))
						request->error = jsrpc_ERROR_INVALID_REQUEST;
				}
			}
			break;
		}
		case yajl_t_object:
			request_list->nrequests = 1;
			request_list->requests = cx_alloc(sizeof(RPC_Request));
			RPC_Request* request = request_list->requests;
			request->data = json_root;
			if (!RPC_Request_parse(request))
				request->error = jsrpc_ERROR_INVALID_REQUEST;
			break;
		default:
			return 0;
		}
	}
	else
	{
		request_list->nrequests = -1;
		XFDBG("%s", errbuf);
	}

	return request_list->nrequests;
}

inline void
RPC_RequestList_free_data(RPC_RequestList* pipeline)
{
	yajl_tree_free((yajl_val)pipeline->data);
}
