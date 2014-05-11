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
			fprintf(stderr, "Param index[%d] out of bounds\n", param->pos);
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
			fprintf(stderr, "Deserialized parameter[%d] %s --> %s\n",
				param->pos, param->name, value->u.string);
			return value->u.string;
		default:
		{
			fprintf(stderr, "Parameter [%d:%s] is not a string value.\n",
				param->pos, param->name);
			request->error = jsrpc_ERROR_INVALID_PARAMS;
			break;
		}
		}
	}
	else
		fprintf(stderr, "Missing param[%d] named '%s'\n",
			param->pos, param->name);

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
			fprintf(stderr, "Parameter [%d:%s] is not a longlong value.\n",
				param->pos, param->name);
	}
	else
		fprintf(stderr, "Missing param[%d] named '%s'\n",
			param->pos, param->name);


	return 0;
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
			fprintf(stderr, "Parameter [%d:%s] is not a double value.\n",
				param->pos, param->name);
	}
	else
		fprintf(stderr, "Missing param[%d] named '%s'\n", param->pos, param->name);

	return 0;
}

static int
check_jsonrpc_version(RPC_Request* request, yajl_val root)
{
	yajl_val v = yajl_tree_get(root, JSONRPC_VERSION_PATH, yajl_t_any);

	if (v)
	{
		char* jsonrpc_version = YAJL_GET_STRING(v);

		if (jsonrpc_version)
		{
			if (strcmp(jsonrpc_version, "2.0") == 0)
				return 1;
			else
				fprintf(stderr, "Unsupported jsonrpc version '%s'\n", jsonrpc_version);
		}
		else
			fprintf(stderr, "Invalid type %d for property 'jsonrpc'\n", v->type);
	}
	else
		fprintf(stderr, "No 'jsonrpc' version string\n");

	return 0;
}

/* @return 0 if request is malformed, 1 else */
static int
set_request_id(RPC_Request* request, yajl_val root)
{
	yajl_val v = yajl_tree_get(root, JSONRPC_ID_PATH, yajl_t_any);

	if (v)
	{
		switch (v->type)
		{
		case yajl_t_null:
			fprintf(stderr, "Using null as id is discouraged! Request be handled as notification\n");
			return 0;
		case yajl_t_number:
			request->id = YAJL_GET_NUMBER(v);
			break;
		case yajl_t_string:
			request->id = YAJL_GET_STRING(v);
			break;
		default:
			printf("Invalid type %d for property id\n", v->type);
			return 0;
		}
	}
	else
		printf("Received a notification\n");

	return 1;
}

/* @return 0 if request is malformed, 1 else */
static int
set_request_method(RPC_Request* request, yajl_val root)
{
	yajl_val v = yajl_tree_get(root, JSONRPC_METHOD_PATH, yajl_t_any);

	if (v)
	{
		request->method_name = YAJL_GET_STRING(v);
		if (request->method_name)
			return 1;
		else
			fprintf(stderr, "Invalid type %d for parameter 'method'\n", v->type);
	}
	else
		fprintf(stderr, "Request has no parameter 'method'\n");
	return 0;
}

/* @return 0 if request is malformed, 1 else */
static int
RPC_Request_parse(RPC_Request* request)
{
	int ret;
	yajl_val root = (yajl_val)request->data;

	ret = check_jsonrpc_version(request, root);
	if (!ret)
		return 0;

	ret = set_request_id(request, root);
	if (!ret)
		return 0;

	ret = set_request_method(request, root);
	if (!ret)
		return 0;

	return 1;
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
				request_list->requests = calloc((size_t)request_list->nrequests, sizeof(RPC_Request));
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
			request_list->requests = calloc(1, sizeof(RPC_Request));
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
