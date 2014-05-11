#include "echo_service.h"

RPC_method(echo)
{
	StringBuffer_printf(result_buffer, "\"%s\"", RPC_get_param(echo, input));
}

RPC_method(echo_double)
{
	StringBuffer_printf(result_buffer, "%lf", RPC_get_param(echo_double, input));
}

RPC_method(echo_longlong)
{
	StringBuffer_printf(result_buffer, "%lld", RPC_get_param(echo_longlong, input));
}
