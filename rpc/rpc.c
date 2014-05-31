#include "rpc.h"

void
cx_rpc_free_simple(void* object)
{
	cx_free(object);
}

void
RPC_Request_init(RPC_Request* rpc_request, Request* request)
{
	// clears everything (including the result)
	memset(rpc_request, 0, sizeof(RPC_Request));
	rpc_request->request = request;
}

void
RPC_Result_set_error(RPC_Result* result, RPC_Error err_code, const char* err_message)
{
	assert(result->error == CX_RPC_ERROR_OK);
	assert(err_code != CX_RPC_ERROR_OK);

	result->error = err_code;
	if (err_message)
	{
		assert(result->value.type == RPC_TYPE_VOID);
		result->value.type = RPC_TYPE_STRING;
		result->value.data.string = cx_strndup(err_message, RPC_ERROR_MESSAGE_LENGTH_MAX);
		result->value.f_free = &cx_rpc_free_simple;
	}
}

const char*
cx_rpc_strerror(RPC_Error err)
{
	switch (err)
	{
	case CX_RPC_ERROR_OK: return "OK";
	case CX_RPC_ERROR_PARSE: return "Parse error";
	case CX_RPC_ERROR_INVALID_REQUEST: return "Invalid Request";
	case CX_RPC_ERROR_METHOD_NOT_FOUND: return "Method not found";
	case CX_RPC_ERROR_INVALID_PARAMS: return "Invalid params";
	case CX_RPC_ERROR_INTERNAL: return "Internal error";
	}
}

RPC_Value*
Param_get(RPC_Param* params, int position, const char* name, int num_params)
{
	if (params)
	{
		int index;
		for (index = 0; index < num_params; index++)
		{
			RPC_Param* param = &params[index];
			if (param->position == position)
				return &param->value;
			else if (param->name != NULL && strcmp(param->name, name) == 0)
				return &param->value;
		}
	}
	return NULL;
}

void
Service_call(RPC_MethodTable* service_methods, RPC_Request* request)
{
	cx_err_clear(); /* clear previous errors */

	RPC_MethodTable* wrapped_method = service_methods;
	bool method_missing = true;

	/* stop when method name is NULL */
	while (wrapped_method->method_name)
	{
		if (strcmp(wrapped_method->method_name, request->method_name) == 0)
		{
			XFDBG("Calling method[%s] (wrapper:%p, params:%d, format:%d)",
			      request->method_name, (void*)wrapped_method->method_wrapper, request->num_params, request->format);
			wrapped_method->method_wrapper(request->params, request->num_params, &request->result, request->format);
			method_missing = false;
			break;
		}
		wrapped_method++;
	}

	/* free allocated param values */
	int i;
	RPC_Param* param = request->params;
	for (i = 0; i < request->num_params; i++)
	{
		if (param->value.f_free)
			param->value.f_free(param->value.data.object);

		param++;
	}

	if (method_missing)
		RPC_Result_set_error(&request->result, CX_RPC_ERROR_METHOD_NOT_FOUND, NULL);

	if (cx_err_code)
	{
		if (cx_err_code < JSON_RPC_ERROR_MIN || cx_err_code > JSON_RPC_ERROR_MAX)
		{
			RPC_Result_set_error(&request->result, CX_RPC_ERROR_INTERNAL, NULL);
		}
		else
		{
			RPC_Result_set_error(&request->result, cx_err_code, NULL);
		}
	}
}

void
RPC_Request_free(RPC_Request* request)
{
	if (request->result.value.f_free)
		request->result.value.f_free(request->result.value.data.object);

	if (request->params)
		cx_free(request->params);

	memset(request, 0, sizeof(RPC_Request));
}
