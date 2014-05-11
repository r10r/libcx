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

static yajl_val
get_param_value(RPC_Request* request, RPC_Param* param)
{
	yajl_val params = yajl_tree_get((yajl_val)request->userdata, JSONRPC_PARAMS_PATH, yajl_t_any);

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
set_request_id(RPC_Request* request, yajl_val root)
{
	yajl_val v = yajl_tree_get(root, JSONRPC_ID_PATH, yajl_t_any);

	if (v)
	{
		switch (v->type)
		{
		case yajl_t_null:
			fprintf(stderr, "Using null as id is discouraged!\n");
			return 0;
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
		// TODO return invalid request
		fprintf(stderr, "Request has no parameter 'method'\n");
	return 0;
}

/* @returns 0 if request is malformed, 1 else */
static int
RPC_Request_parse(RPC_Request* request)
{
	int ret;
	yajl_val root = (yajl_val)request->userdata;

	printf("YYYYYY: %p obj:%d\n", root, YAJL_IS_OBJECT(root));

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

/*
 * @return the number of requests (> 0) or 0 if the input is invalid
 * @malloc request (count * sizeof(Request))
 */
static size_t
RPC_Request_deserialize(Pipeline* pipeline)
{
	char errbuf[1024];
	size_t nrequests = 0;

	yajl_val root = yajl_tree_parse(StringBuffer_value(pipeline->request_buffer), errbuf, sizeof(errbuf));

	pipeline->userdata = root;

	printf("YYYYYY: %p obj:%d arr:%d\n", root, YAJL_IS_OBJECT(root), YAJL_IS_ARRAY(root));

	if (root)
	{
		switch (root->type)
		{
		case yajl_t_array:
		{
			nrequests = root->u.array.len;
			pipeline->requests = calloc(nrequests, sizeof(RPC_Request));
			size_t i;
			for (i = 0; i < nrequests; i++)
			{
				RPC_Request* request = pipeline->requests + i;
				request->userdata = root->u.array.values[i];
				RPC_Request_parse(request);
			}
			break;
		}
		case yajl_t_object:
			nrequests = 1;
			pipeline->requests = calloc(1, sizeof(RPC_Request));
			RPC_Request* request = pipeline->requests;
			request->userdata = root;
			RPC_Request_parse(request);
			break;
		default:
			fprintf(stderr, "Invalid request (neither object/nor array)\n");
		}
	}
	else
		fprintf(stderr, "Invalid request. Failed to parse request: \n%s\n", errbuf);

	return nrequests;
}

Pipeline*
RPC_Pipeline_new()
{
	Pipeline* pipeline = calloc(1, sizeof(Pipeline));

	pipeline->request_buffer = StringBuffer_new(2048);
	pipeline->response_buffer = StringBuffer_new(2048);
	pipeline->result_buffer = StringBuffer_new(2048);

	return pipeline;
}

void
RPC_Pipeline_free(Pipeline* pipeline)
{
	// destroy pipeline (TODO recycle pipeline for next request | pooling)
	yajl_tree_free((yajl_val)pipeline->userdata);
	free(pipeline->requests);
	StringBuffer_free(pipeline->result_buffer);
	StringBuffer_free(pipeline->request_buffer);
	StringBuffer_free(pipeline->response_buffer);
	free(pipeline);
}

/*
 * A batch request is just a list of independent requests (no atomicity)
 * - the service can decide whether all requests in a batch request must
 *       be not-malformed for any request to be processed
 *
 * process request
 * - a request writes the result to the result buffer
 * - after the request has been processed the result buffer is
 *      appended to the response buffer
 *
 *      - multiple requests of a batch-request are always passed serially
 *              to the service
 *      - the service might multiplex multiple requests (e.g mpd_command_list_begin/end)
 *
 *      TODO test mpd command list behaviour
 */
void
RPC_Pipeline_process(Pipeline* pipeline, RPC_Method methods[])
{
	printf("Dispatching request: \n<<<\n%s\n>>>\n", StringBuffer_value(pipeline->request_buffer));

	pipeline->nrequests = RPC_Request_deserialize(pipeline);
	printf("Requests: %zu\n", pipeline->nrequests);

	// begin batch response
	if (pipeline->nrequests > 1)
		StringBuffer_ncat(pipeline->response_buffer, "[", 1);

	size_t i;
	for (i = 0; i < pipeline->nrequests; i++)
	{
		RPC_Request* request = pipeline->requests + i;
		// lookup service method
		request->method = RPC_Request_lookup_method(request, methods);

		if (request->method)
			request->method->method(request, pipeline->result_buffer);
		else
		{
			// method not found
			StringBuffer_printf(pipeline->result_buffer, JSONRPC_ERROR_SIMPLE,
					    jsrpc_ERROR_METHOD_NOT_FOUND, "Method not found");
		}

		// append response with result or error to response buffer
		if (request->id)
		{
			if (i > 0 && pipeline->nrequests > 1)
				StringBuffer_ncat(pipeline->response_buffer, ",", 1);

			if (StringBuffer_used(pipeline->result_buffer) == 0)
				// no result
				StringBuffer_aprintf(pipeline->response_buffer,
						     JSONRPC_RESPONSE_NULL, request->id);
			else
				StringBuffer_aprintf(pipeline->response_buffer,
						     JSONRPC_RESPONSE_SIMPLE, request->id, StringBuffer_value(pipeline->result_buffer));

			// clear result buffer after each request
			StringBuffer_clear(pipeline->result_buffer);
		}
	}

	// end batch response
	if (pipeline->nrequests > 1)
		StringBuffer_ncat(pipeline->response_buffer, "]", 1);

	// return response buffer
	printf("Response: \n<<<\n%s\n>>>\n", StringBuffer_value(pipeline->response_buffer));
}
