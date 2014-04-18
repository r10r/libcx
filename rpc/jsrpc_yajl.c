#include "jsrpc.h"
#include "list/list.h"
#include "yajl/yajl_tree.h"

static RPC_Method*
lookup_method(RPC_Method methods[], RPC_Request* request)
{
	int i;

	for (i = 0; methods[i].name != NULL; i++)
		if (strcmp(methods[i].name, request->method) == 0)
			return &methods[i];

	return NULL;
}

void*
get_param_value_string(RPC_Param* param, int index, void* data)
{
	yajl_val params = (yajl_val)data;
	yajl_val value = NULL;


	if (YAJL_IS_ARRAY(params))
	{
		// check bounds
		if (index < (int)params->u.array.len)
			value = *(params->u.array.values + index);
		else
			fprintf(stderr, "Param index[%d] out of bounds\n", index);
	}
	else if (YAJL_IS_OBJECT(params))
	{
		const char* parameter_path[] = { param->name, NULL };
		value = yajl_tree_get(params, parameter_path, yajl_t_any);
	}

	if (value)
	{
		switch (value->type)
		{
		case yajl_t_null:
			break;
		case yajl_t_string:
			fprintf(stderr, "Deserialized parameter[%d] %s --> %s\n", index, param->name, value->u.string);
			return value->u.string;
		default:
			printf("Invalid type %d for property id\n", value->type);
			// TODO return invalid request with null id if not notification
			break;
		}
	}
	else
		fprintf(stderr, "Missing param[%d] named '%s'\n", index, param->name);

	return NULL;
}

static int
extract_request_parameters(yajl_val v, RPC_Request* request, RPC_Method* method)
{
	if (method->param_count == 0)
	{
		if (v || v->type != yajl_t_null)
			// warn if params are set even if method does not require params
			fprintf(stderr, "Method [%s] has no parameters. Params ignored\n", method->name);
		return 1;
	}
	else
	{
		if (!v)
			// TODO log missing parameters
			printf("Missing parameters\n");
		else
		{
			size_t length = 0;

			if (YAJL_IS_ARRAY(v))
				length = v->u.array.len;
			else if (YAJL_IS_OBJECT(v))
				length = v->u.object.len;
			else
			{
				fprintf(stderr, "Invalid type for value 'params' %d\n", v->type);
				return 0;
			}

			if (length < (size_t)method->param_count - 1)
				fprintf(stderr, "Missing parameters. Only %zu of %d parameters provided\n", length, method->param_count);
			else
			{
				// TODO handle optional parameters
				int i = 0;
				for (i = 0; i < method->param_count; i++)
					request->params[i] = (char*)method->signature[i].deserialize(&method->signature[i], i, v);
				return 1;
			}
		}
	}
	return 0;
}

static int
check_jsonrpc_version(yajl_val v)
{
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

static int
parse_request_id(yajl_val v, RPC_Request* request)
{
	if (v)
	{
		switch (v->type)
		{
		case yajl_t_null:
			fprintf(stderr, "Using null as id is discouraged!\n");
			break;
		case yajl_t_number:
			request->id = v->u.number.r;
			break;
		case yajl_t_string:
			request->id = v->u.string;
			break;
		default:
			printf("Invalid type %d for property id\n", v->type);
			// TODO return invalid request with null id if not notification
			return 0;
		}
	}
	else
		// its' a notification
		printf("Request has no id - it must be a notification\n");
	return 1;
}

static int
parse_request_method(yajl_val v, RPC_Request* request)
{
	if (v)
	{
		request->method = YAJL_GET_STRING(v);
		if (request->method)
			return 1;
		else
			fprintf(stderr, "Invalid type %d for parameter 'method'\n", v->type);
	}
	else
		// TODO return invalid request
		fprintf(stderr, "Request has no method parameter\n");
	return 0;
}

void
dispatch_request(RPC_Method methods[], const char* json)
{
	char errbuf[1024];

	yajl_val root = yajl_tree_parse(json, errbuf, sizeof(errbuf));

	if (!root)
		fprintf(stderr, "Invalid request. Failed to parse request: \n%s\n", errbuf);

	RPC_Request request;
	const char* jsonrpc_path[] = { "jsonrpc", NULL };
	const char* id_path[] = { "id", NULL };
	const char* method_path[] = { "method", NULL };
	const char* params_path[] = { "params", NULL };

	yajl_val v;
	int ret;

	ret = check_jsonrpc_version(yajl_tree_get(root, jsonrpc_path, yajl_t_any));
	if (!ret)
		goto error;         // TODO send error

	ret = parse_request_id(yajl_tree_get(root, id_path, yajl_t_any), &request);
	if (!ret)
		goto error; // TODO send error

	ret = parse_request_method(yajl_tree_get(root, method_path, yajl_t_any), &request);
	if (!ret)
		goto error; // TODO send error

	printf("--> Request id:%s method:%s\n", request.id, request.method);

	RPC_Method* method = lookup_method(methods, &request);
	if (!method)
	{
		fprintf(stderr, "Method [%s] does not exist\n", request.method);
		goto error; // TODO send error
	}

	ret = extract_request_parameters(yajl_tree_get(root, params_path, yajl_t_any), &request, method);
	if (!ret)
		goto error; // TODO send error

	// if we want to process the request evented we have to do a memcpy the request here
	// values saved to the request must be duplicated !!!
	method->method(&request);

error:
	yajl_tree_free(root);
}
