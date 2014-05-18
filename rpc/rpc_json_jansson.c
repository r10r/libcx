#include "rpc_json.h"
#include <jansson.h>

static void
deserialize_param_int(Param* param, json_t* value)
{
	long long long_val = json_integer_value(value);

	if (long_val <= INT_MAX && long_val >= INT_MIN)
	{
//		XFDBG("[%d](%s): (int)%d", index, key, (int)long_val);
		param->value.type = TYPE_INTEGER;
		param->value.value.integer = (int)long_val;
	}
	else
	{
//		XFDBG("[%d](%s): (long long)%lld", index, key, long_val);
		param->value.type = TYPE_LONGLONG;
		param->value.value.longlong = long_val;
	}
}

static void
deserialize_param(Param* param, int position, const char* key, json_t* value)
{
	param->position = position;
	param->name = key;

	if (json_is_integer(value))
	{
		deserialize_param_int(param, value);
	}
}

static int
params_from_object(Param** params, json_t* json)
{
	size_t num_params = json_object_size(json);

	*params = cx_alloc(num_params * sizeof(Param));
	void* iter = json_object_iter(json);
	Param* param = params[0];
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
params_from_array(Param** params, json_t* json)
{
	size_t num_params = json_array_size(json);

	*params = cx_alloc(num_params * sizeof(Param));

	Param* param = params[0];
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
Params_from_json(Param** params, json_t* json)
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
		set_cx_errno(ERROR_PARAM_UNSUPPORTED_FORMAT);
		return -1;
	}
}

json_t*
Value_to_json(Value* value)
{
	switch (value->type)
	{
	case TYPE_INTEGER:
		return json_integer(value->value.integer);
	case TYPE_LONGLONG:
		return json_integer(value->value.longlong);
	case TYPE_DOUBLE:
		return json_real(value->value.floatingpoint);
	case TYPE_STRING:
		return json_string(value->value.string);
	case TYPE_BOOLEAN:
		return json_boolean(value->value.boolean);
	case TYPE_NULL:
		return json_null();
	case TYPE_OBJECT:
		if (value->f_to_json)
			return value->f_to_json(value->value.object);
		else
			set_cx_errno(ERROR_METHOD_MISSING);
	}
	return NULL;
}
