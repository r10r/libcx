#include "echo_service.h"
#include <jansson.h>

//static json_t*
static const char*
obj_to_json(RPC_Result* result)
{
//	return json_string(StringBuffer_value((StringBuffer*)result->obj));
	return (const char*)StringBuffer_value((StringBuffer*)result->obj);
}

static const char*
obj_to_s(RPC_Result* result)
{
	return (const char*)StringBuffer_value((StringBuffer*)result->obj);
}

static RPC_Result*
RPC_Result_buff(StringBuffer* buff)
{
	RPC_Result* result = RPC_Result_new(buff);

	result->f_free_obj = (F_ResultFree*)StringBuffer_free;
	result->f_to_json = (F_ResultToJSON*)obj_to_json;
	result->f_to_s = obj_to_s;
	return result;
}

#define RPC_Result_printf(fmt, ...) \
	RPC_Result_buff(StringBuffer_from_printf(1024, fmt, __VA_ARGS__))

RPC_single_string_param(echo, input, 0)
RPC_define_with_params(echo)
{
	return RPC_Result_printf("\"%s\"", RPC_get_param(echo, input));
}

RPC_single_double_param(echo_double, input, 0)
RPC_define_with_params(echo_double)
{
	return RPC_Result_printf("%.3lf", RPC_get_param(echo_double, input));
}

RPC_single_longlong_param(echo_longlong, input, 0)
RPC_define_with_params(echo_longlong)
{
	return RPC_Result_printf("%lld", RPC_get_param(echo_longlong, input));
}
