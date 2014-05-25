#include "rpc.h"

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
			param->value.f_free(param->value.value.object);

		param++;
	}

	if (method_missing)
		request->error = CX_RPC_ERROR_METHOD_NOT_FOUND;
	else if (cx_err_code != CX_ERR_OK)
		request->error = cx_err_code;
}
