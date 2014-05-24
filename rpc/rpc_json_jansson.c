#include "rpc_json.h"
#include <jansson.h>

static void
deserialize_param_int(RPC_Param* param, json_t* value)
{
	long long long_val = json_integer_value(value);

	if (long_val <= INT_MAX && long_val >= INT_MIN)
	{
//		XFDBG("[%d](%s): (int)%d", index, key, (int)long_val);
		param->value.type = RPC_TYPE_INTEGER;
		param->value.value.integer = (int)long_val;
	}
	else
	{
//		XFDBG("[%d](%s): (long long)%lld", index, key, long_val);
		param->value.type = RPC_TYPE_LONGLONG;
		param->value.value.longlong = long_val;
	}
}

static void
deserialize_param(RPC_Param* param, int position, const char* key, json_t* value)
{
	param->position = position;
	param->name = key;

	if (json_is_integer(value))
	{
		deserialize_param_int(param, value);
	}
}

static int
params_from_object(RPC_Param** params, json_t* json)
{
	size_t num_params = json_object_size(json);

	*params = cx_alloc(num_params * sizeof(RPC_Param));
	void* iter = json_object_iter(json);
	RPC_Param* param = params[0];
	size_t index = 0;
	while (iter != NULL)
	{
		const char* key = json_object_iter_key(iter);
		json_t* value = json_object_iter_value(iter);

		deserialize_param(param, -1, key, value);

		iter = json_object_iter_next(json, iter);
		param++;
		index++;
	}
	return (int)num_params;
}

static int
params_from_array(RPC_Param** params, json_t* json)
{
	size_t num_params = json_array_size(json);

	*params = cx_alloc(num_params * sizeof(RPC_Param));

	RPC_Param* param = params[0];
	json_t* value = NULL;
	size_t index;
	json_array_foreach(json, index, value)
	{
		deserialize_param(param, (int)index, NULL, value);
		param++;
	}

	return (int)num_params;
}

int
Params_from_json(RPC_Param** params, json_t* json)
{
	if (json_is_object(json))
	{
		return params_from_object(params, json);
	}
	else if (json_is_array(json))
	{
		return params_from_array(params, json);
	}
	else
	{
		XERR("parameter 'params': invalid format (expected array or object)");
		set_cx_errno(RPC_ERROR_PARAM_INVALID_TYPE);
		return -1;
	}
}

json_t*
Value_to_json(RPC_Value* value)
{
	switch (value->type)
	{
	case RPC_TYPE_INTEGER:
		return json_integer(value->value.integer);
	case RPC_TYPE_LONGLONG:
		return json_integer(value->value.longlong);
	case RPC_TYPE_DOUBLE:
		return json_real(value->value.floatingpoint);
	case RPC_TYPE_STRING:
		return json_string(value->value.string);
	case RPC_TYPE_BOOLEAN:
		return json_boolean(value->value.boolean);
	case RPC_TYPE_NULL:
		return json_null();
	case RPC_TYPE_OBJECT:
		if (value->f_to_json)
			return value->f_to_json(value->value.object);
		else
			set_cx_errno(RPC_ERROR_METHOD_MISSING);
	}
	return NULL;
}
