#include "echo_service.h"

RPC(method, echo)
{
	StringBuffer_cat(result_buffer, RPC(get_param, echo, input));
}

RPC(method, echo_double)
{
	StringBuffer_printf(result_buffer, "%lf", RPC(get_param, echo_double, input));
}

RPC(method, echo_longlong)
{
	StringBuffer_printf(result_buffer, "%lld", RPC(get_param, echo_longlong, input));
}
