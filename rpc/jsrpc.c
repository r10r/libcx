#include "jsrpc.h"

const char*
jsrpc_strerror(int code)
{
	switch (code)
	{
	case jsrpc_ERROR_PARSE_ERROR: return "Parse error";
	case jsrpc_ERROR_INVALID_REQUEST: return "Invalid Request";
	case jsrpc_ERROR_METHOD_NOT_FOUND: return "Method not found";
	case jsrpc_ERROR_INVALID_PARAMS: return "Invalid params";
	case jsrpc_ERROR_INTERNAL: return "Internal error";
	default: return "Undefined error";
	}
}

static RPC_Method*
RPC_RequestList_deserialize_method(RPC_RequestList* request_list, RPC_Request* request, RPC_Method* methods)
{
	/* lookup service method */
	RPC_Method* rpc_method = RPC_Request_lookup_method(request, methods);

	if (!rpc_method)
	{
		/* method not found */
		request->error = jsrpc_ERROR_METHOD_NOT_FOUND;
		StringBuffer_cat(request_list->result_buffer,
				 jsrpc_strerror(jsrpc_ERROR_METHOD_NOT_FOUND));
	}
	else
	{
		int nparams = rpc_method->param_count;
		if (nparams > 0)
		{
			/* deserialize the parameters */
			request->params = cx_alloc((size_t)nparams * sizeof(RPC_Value));
			int i;
			for (i = 0; i < nparams; i++)
			{
				RPC_Param* param = rpc_method->signature[i];
				param->f_deserialize(request);

				/* check if parameter was successfully deserialized */
				if (request->error)
				{
					StringBuffer_cat(request_list->result_buffer,
							 jsrpc_strerror(jsrpc_ERROR_INVALID_PARAMS));
					return NULL;
				}
			}
		}
	}
	return rpc_method;
}

static char*
RPC_Request_serialize_id(RPC_Request* request)
{
	char* id = NULL;

	if (request->id == NULL)
	{
		id = cx_strdup(JSONRPC_NULL);
	}
	else
	{
		size_t max_size = (strlen(request->id) + 3) * sizeof(char);
		id = cx_alloc(max_size);
		if (request->id_numerical)
			snprintf(id, max_size, "%s", request->id);
		else
			snprintf(id, max_size, "\"%s\"", request->id);
	}
	return id;
}

static void
RPC_RequestList_process_response(RPC_RequestList* request_list, RPC_Request* request, const char* id, RPC_Result* result)
{
	request->response_written = 1;

	/* check for execution error */
	if (request->error)
	{
		StringBuffer_aprintf(request_list->response_buffer, JSONRPC_RESPONSE_ERROR,
				     id, request->error,
				     StringBuffer_value(request_list->result_buffer));
	}
	else
	{
		// FIXME use to_json method here
		const char* data = result->f_to_s(result);
		if (strlen(data) > 0)
			StringBuffer_aprintf(request_list->response_buffer, JSONRPC_RESPONSE_SIMPLE, id, data);
		else
			StringBuffer_aprintf(request_list->response_buffer, JSONRPC_RESPONSE_NULL, id);
	}
}

static void
RPC_RequestList_process_request(RPC_RequestList* request_list, int nrequest, RPC_Method methods[])
{
	RPC_Request* request = request_list->requests + nrequest;

	char* id = RPC_Request_serialize_id(request);

	/* check for deserialization errors (only invalid request) */
	if (request->error)
	{
		StringBuffer_aprintf(request_list->response_buffer, JSONRPC_RESPONSE_ERROR,
				     id, request->error, jsrpc_strerror(request->error));
		request->response_written = 1;
	}
	else
	{
		/* deserialize the parameters and call the method */
		if (request->method_name)
		{
			RPC_Method* method = RPC_RequestList_deserialize_method(request_list, request, methods);
			if (method)
			{
				RPC_Result* result = method->method(request->params);

				/* append result to response buffer if request is not a notification */
				if (request->id)
				{
					RPC_RequestList_process_response(request_list, request, id, result);
				}
				RPC_Result_free(result);
			}
			else
			{
				StringBuffer_aprintf(request_list->response_buffer, JSONRPC_RESPONSE_ERROR,
						     request->id, request->error, StringBuffer_value(request_list->result_buffer));
			}

			/* clear params */
			cx_free(request->params);
		}
	}

	cx_free(id);
	/* clear result buffer after each request */
	StringBuffer_clear(request_list->result_buffer);
}

static inline void
append_delimiter(RPC_RequestList* request_list, int i)
{
	if ((i > 0 && i < request_list->nrequests) &&
	    (request_list->requests + i - 1)->response_written)
		StringBuffer_ncat(request_list->response_buffer, ",", 1);
}

void
RPC_RequestList_process(RPC_RequestList* request_list, RPC_Method methods[])
{
	XFDBG("Dispatching request: \n<<<\n%s\n>>>", (const char*)request_list->data);

	RPC_Request_deserialize(request_list);

	/* check error */
	if (request_list->nrequests == -1)
	{
		/* parse error */
		StringBuffer_printf(request_list->response_buffer,
				    JSONRPC_RESPONSE_ERROR, JSONRPC_NULL,
				    jsrpc_ERROR_PARSE_ERROR, jsrpc_strerror(jsrpc_ERROR_PARSE_ERROR));
	}
	else if (request_list->nrequests == 0)
	{
		/* invalid request object */
		StringBuffer_printf(request_list->response_buffer,
				    JSONRPC_RESPONSE_ERROR, JSONRPC_NULL,
				    jsrpc_ERROR_INVALID_REQUEST, jsrpc_strerror(jsrpc_ERROR_INVALID_REQUEST));
	}
	else if (request_list->nrequests > 0)
	{
		if (request_list->batch)
		{
			/* begin batch response */
			StringBuffer_ncat(request_list->response_buffer, "[", 1);

			int i;
			for (i = 0; i < request_list->nrequests; i++)
			{
				append_delimiter(request_list, i);
				RPC_RequestList_process_request(request_list, i, methods);
			}

			/* end batch response */
			StringBuffer_ncat(request_list->response_buffer, "]", 1);
		}
		else
			RPC_RequestList_process_request(request_list, 0, methods);
	}

	/* return response buffer */
	XFDBG("Response: \n<<<\n%s\n>>>", StringBuffer_value(request_list->response_buffer));
}
