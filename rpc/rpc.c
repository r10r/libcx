#include "rpc.h"

RPC_Value*
Param_get(RPC_Param* params, int position, const char* name, int num_params)
{
	if (!params)
	{
		cx_errno_set(RPC_ERROR_NO_PARAMS);
	}
	else
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

int
Service_call(RPC_MethodTable* service_methods, RPC_Request* request)
{
	cx_errno_set(0); /* clear previous errors */
	int status = 1;

	RPC_MethodTable* wrapped_method = service_methods;
	bool method_missing = true;

	while (wrapped_method->method_name)
	{
		if (strcmp(wrapped_method->method_name, request->method_name) == 0)
		{
			XFDBG("Calling method[%s] (wrapper:%p, params:%d, format:%d)",
			      request->method_name, (void*)wrapped_method->method_wrapper, request->num_params, request->format);
			status = wrapped_method->method_wrapper(request->params, request->num_params, &request->result, request->format);
			method_missing = false;
			break;
		}
		wrapped_method++;
	}

	if (method_missing)
		cx_errno_set(RPC_ERROR_METHOD_MISSING);

	/* free allocated param values */
	int i;
	RPC_Param* param = request->params;
	for (i = 0; i < request->num_params; i++)
	{
		if (param->value.f_free)
			param->value.f_free(param->value.value.object);

		param++;
	}

	if (cx_errno == 0)
		return status;
	else
	{
		request->error = (RPC_Error)cx_errno;   // FIXME ensure only RPC errors are written to cx_errno, use a wrapper macro
		XFERR("ERROR while calling method '%s' (cx_errno %d)", request->method_name, cx_errno);
		return -1;
	}
}
