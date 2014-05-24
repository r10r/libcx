#include "rpc.h"

Value*
Param_get(Param* params, int position, const char* name, int num_params)
{
	if (!params)
	{
		set_cx_errno(ERROR_NO_PARAMS);
	}
	else
	{
		int index;
		for (index = 0; index < num_params; index++)
		{
			Param* param = &params[index];
			if (param->position == position)
				return &param->value;
			else if (param->name != NULL && strcmp(param->name, name) == 0)
				return &param->value;
		}
	}
	return NULL;
}

/* @return
 *      -1 on error see cx_errno
 *      0 method has void return,
 *      1 if return values is valid
 */
int
Service_call(MethodTable* service_methods, RPC_Request* request)
{
	set_cx_errno(0); /* clear previous errors */
	int status = 1;

	MethodTable* wrapped_method = service_methods;
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
		set_cx_errno(ERROR_METHOD_MISSING);

	/* free allocated param values */
	int i;
	Param* param = request->params;
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
		XFERR("ERROR while calling method '%s' (cx_errno %d)", request->method_name, cx_errno);
		return -1;
	}
}
