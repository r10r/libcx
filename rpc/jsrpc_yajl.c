#include "jsrpc.h"
#include "list/list.h"
#include "yajl/yajl_tree.h"

static const char* JSONRPC_VERSION_PATH[] = { "jsonrpc", NULL };
static const char* JSONRPC_ID_PATH[] = { "id", NULL };
static const char* JSONRPC_METHOD_PATH[] = { "method", NULL };
static const char* JSONRPC_PARAMS_PATH[] = { "params", NULL };

enum request_type
{
	REQUEST_NOTIFICATION,
	REQUEST_SINGLE,
	REQUEST_BATCH
};

typedef struct request_status_t
{
	enum request_type type;

	int last_error;
	enum request_type last; /* previous response in batch request */
	yajl_val root;
	yajl_val req_root;
} RequestStatus;

#define status_of(request) \
	((RequestStatus*)(request)->userdata)


static RPC_Method*
lookup_method(RPC_Method methods[], RPC_Request* request)
{
	int i;

	for (i = 0; methods[i].name != NULL; i++)
		if (strcmp(methods[i].name, request->method) == 0)
			return &methods[i];

	return NULL;
}

static yajl_val
get_param_value(RPC_Request* request, RPC_Param* param)
{
	yajl_val params = yajl_tree_get(status_of(request)->req_root, JSONRPC_PARAMS_PATH, yajl_t_any);

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
			fprintf(stderr, "Parameter [%d:%s] is not a string value.\n",
				param->pos, param->name);
			// TODO return invalid request with null id if not notification
			break;
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

static int
parse_request_id(RPC_Request* request, yajl_val root)
{
	yajl_val v = yajl_tree_get(root, JSONRPC_ID_PATH, yajl_t_any);

	if (v)
	{
		switch (v->type)
		{
		case yajl_t_null:
			fprintf(stderr, "Using null as id is discouraged!\n");
			break;
		case yajl_t_number:
			request->id = YAJL_GET_NUMBER(v);
			break;
		case yajl_t_string:
			request->id = YAJL_GET_STRING(v);
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
parse_request_method(RPC_Request* request, yajl_val root)
{
	yajl_val v = yajl_tree_get(root, JSONRPC_METHOD_PATH, yajl_t_any);

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

/* returns -1 on malformed request, 0 if method does not exist, 1 if request was processed */
static int
RPC_Request_process(RPC_Request* request, RPC_Method methods[], RequestStatus* status)
{
	int ret;

	ret = check_jsonrpc_version(request, status->req_root);
	if (!ret)
		return -1;

	ret = parse_request_id(request, status->req_root);
	if (!ret)
		return -1;

	ret = parse_request_method(request, status->req_root);
	if (!ret)
		return -1;

	printf("--> Request id:%s method:%s\n", request->id, request->method);

	if (request->id)
		status->last = REQUEST_SINGLE;
	else
		status->last = REQUEST_NOTIFICATION;

	RPC_Method* method = lookup_method(methods, request);
	if (method)
	{
		printf("Dispatching request to method %s %p\n", method->name, method->method);
		method->method(request);
		return 1;
	}
	else
	{
		fprintf(stderr, "Method [%s] does not exist\n", request->method);
		return 0;
	}
}

// TODO wrap RPC request in JSONRPC request ?
// ([{"jsonrpc":"2.0",id:}])

#define JSON_MIN_RESPONSE_SIZE 24

void
RPC_Request_dispatch(RPC_Request* request, RPC_Method methods[])
{
	char errbuf[1024];

	RequestStatus request_status;

	memset(&request_status, 0, sizeof(RequestStatus));
	request->userdata = &request_status;

	printf("Dispatching request: \n<<<\n%s\n>>>\n", StringBuffer_value(&request->request_buffer));
	yajl_val root = yajl_tree_parse(StringBuffer_value(&request->request_buffer), errbuf, sizeof(errbuf));
	request_status.root = root;

	if (request_status.root)
	{
		switch (request_status.root->type)
		{
		case yajl_t_array:
			// batch request
		{
			request_status.type = REQUEST_BATCH;
			// every method either returns a result, an error or nothing at all (a notification)
			// start batch response
			// TODO ? check if batch contains only notifications ? we don't have to return nothing
			size_t i;
			jsrpc_begin_batch;
			for (i = 0; i < request_status.root->u.array.len; i++)
			{
				// TODO check if previous request returned something (result/error)
				if (i > 0 && request_status.last == REQUEST_SINGLE)
					jsrpc_write_append_simple(",");

				request_status.req_root = request_status.root->u.array.values[i];
				RPC_Request_process(request, methods, &request_status);
			}
			jsrpc_end_batch;
			break;
		}
		case yajl_t_object:
			request_status.type = REQUEST_SINGLE;
			request_status.req_root = request_status.root;
			RPC_Request_process(request, methods, &request_status);
			break;
		default:
			fprintf(stderr, "Invalid request. Failed to parse request: \n%s\n", errbuf);
		}
	}
	else
		fprintf(stderr, "Invalid request. Failed to parse request: \n%s\n", errbuf);
	// TODO send error

	fprintf(stderr, "Response: \n<<<\n%s\n>>>\n", StringBuffer_value(&request->response_buffer));

	// remove empty batch response
	if (StringBuffer_used(&request->response_buffer) < JSON_MIN_RESPONSE_SIZE)
	{
		fprintf(stderr, "Response is to small to be valid. Clearing response buffer\n");
		StringBuffer_clear(&request->response_buffer);
	}

	printf("XXXXXX rs:%p rs->root:%p root:%p\n", &request_status, request_status.root, root);
	yajl_tree_free(request_status.root);
}
