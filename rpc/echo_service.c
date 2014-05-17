#include "echo_service.h"

/* export each function + method definition */

#define Response_printf(fmt, ...) \
	Response_new(StringBuffer_from_printf(1024, fmt, __VA_ARGS__))

RPC_single_string_param(echo, input, 0)
RPC_define_with_params(echo)
{
	return Response_printf("\"%s\"", RPC_get_param(echo, input));
}

RPC_single_double_param(echo_double, input, 0)
RPC_define_with_params(echo_double)
{
	return Response_printf("%.3lf", RPC_get_param(echo_double, input));
}

RPC_single_longlong_param(echo_longlong, input, 0)
RPC_define_with_params(echo_longlong)
{
	return Response_printf("%lld", RPC_get_param(echo_longlong, input));
}
