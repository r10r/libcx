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
Service_call(MethodTable* method_map, const char* method_name, Param* params, int num_params, Value* result, ParamFormat format)
{
	set_cx_errno(0); /* clear previous errors */
	int status = 1;

	MethodTable* mapped_method = method_map;
	bool method_missing = true;

	while (mapped_method->name)
	{
		if (strcmp(mapped_method->name, method_name) == 0)
		{
			XFDBG("Calling method[%s] (wrapper:%p, params:%d, format:%d)",
			      method_name, (void*)mapped_method->method, num_params, format);
			status = mapped_method->method(params, num_params, result, format);
			method_missing = false;
			break;
		}
		mapped_method++;
	}

	if (method_missing)
		set_cx_errno(ERROR_METHOD_MISSING);

	/* free allocated param values */
	int i;
	Param* param = params;
	for (i = 0; i < num_params; i++)
	{
		if (param->value.f_free)
			param->value.f_free(param->value.value.object);

		param++;
	}

	if (cx_errno == 0)
		return status;
	else
	{
		XFERR("ERROR calling method '%s' (cx_errno %d)", method_name, cx_errno);
		return -1;
	}
}
