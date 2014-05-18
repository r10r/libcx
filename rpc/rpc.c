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
