#include "echo_service.h"

/* export each function + method definition */

RPC_single_string_param(echo, 0, input, 0)
RPC_define_with_params(echo)
{
	StringBuffer_printf(result_buffer, "\"%s\"", RPC_get_param(echo, input));
}

RPC_single_double_param(echo_double, 0, input, 0)
RPC_define_with_params(echo_double)
{
	StringBuffer_printf(result_buffer, "%lf", RPC_get_param(echo_double, input));
}

RPC_single_longlong_param(echo_longlong, 0, input, 0)
RPC_define_with_params(echo_longlong)
{
	StringBuffer_printf(result_buffer, "%lld", RPC_get_param(echo_longlong, input));
}
