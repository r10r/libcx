#include "jsrpc.h"
#include "list/list.h"
#include "yajl/yajl_tree.h"

static int
match_method(Node* node, void* key)
{
	RPC_Method* method = (RPC_Method*)node->data;

	return strcmp((const char*)key, method->name);
}

static RPC_Method*
lookup_method(List* methods, RPC_Request* request)
{
	Node* match = List_match(methods, request->method, match_method);

	RPC_Method* method = NULL;

	if (match)
		method = (RPC_Method*)match->data;

	return method;
}

void*
get_param_value_string(jsrpc_Param* param, int index, void* data)
{
	fprintf(stderr, "Deserialize parameter[%d] %s\n", index, param->name);
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

void
dispatch_request(List* methods, const char* json)
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

	/* check version */
	char* version = NULL;

	v = yajl_tree_get(root, jsonrpc_path, yajl_t_any);
	if (v)
	{
		char* jsonrpc_version = YAJL_GET_STRING(v);

		if (jsonrpc_version)
		{
			if (strcmp(jsonrpc_version, "2.0") != 0)
				fprintf(stderr, "Unsupported jsonrpc version '%s'\n", jsonrpc_version);
		}
		else
			fprintf(stderr, "Invalid type %d for property 'jsonrpc'\n", v->type);
	}
	else
		fprintf(stderr, "No 'jsonrpc' version string\n");

	/* parse request id */
	v = yajl_tree_get(root, id_path, yajl_t_any);

	if (v)
	{
		switch (v->type)
		{
		case yajl_t_null:
			fprintf(stderr, "Using null as id is discouraged!\n");
			break;
		case yajl_t_number:
			request.id = v->u.number.r;
			break;
		case yajl_t_string:
			request.id = v->u.string;
			break;
		default:
			printf("Invalid type %d for property id\n", v->type);
			// TODO return invalid request with null id if not notification
			break;
		}
	}
	else
		// its' a notification
		printf("Request has no id - it must be a notification\n");


	/* parse request method */
	v = yajl_tree_get(root, method_path, yajl_t_any);
	if (v)
	{
		request.method = YAJL_GET_STRING(v);
		if (!request.method)
			fprintf(stderr, "Invalid type %d for parameter 'method'\n", v->type);
	}
	else
		// TODO return invalid request
		fprintf(stderr, "Request has no method parameter\n");

	printf("Request id [%s]\n", request.id);
	printf("Method [%s]\n", request.method);

	RPC_Method* method = lookup_method(methods, &request);
	if (!method)
		fprintf(stderr, "Method [%s] does not exist\n", request.method);

	/* extract request parameters */
	v = yajl_tree_get(root, params_path, yajl_t_any);

	if (method->param_count == 0)
	{
		if (v || v->type != yajl_t_null)
			// warn if params are set even if method does not require params
			fprintf(stderr, "Method [%s] has no parameters. Params ignored\n", method->name);
	}
	else
	{
		if (!v)
			// TODO print missing parameters
			printf("Missing parameters\n");
		else
		{
			size_t length = 0;

			if (YAJL_IS_ARRAY(v))
				length = v->u.array.len;
			else if (YAJL_IS_OBJECT(v))
				length = v->u.object.len;
			else
				// FIXME return here
				fprintf(stderr, "Invalid type for value 'params' %d\n", v->type);

			if (length < (size_t)method->param_count - 1)
				// FIXME might be optional parameters
				fprintf(stderr, "Missing parameters. Only %zu of %d parameters provided\n", length, method->param_count);
			else
			{
				int i = 0;
				for (i = 0; i < method->param_count; i++)
					request.params[i] = (char*)method->signature[i].deserialize(&method->signature[i], i, v);
			}
		}
	}

	// if we want to process the request evented we have to do a memcpy the request here
	// values saved to the request must be duplicated !!!
	method->method(&request);

	yajl_tree_free(root);
}
