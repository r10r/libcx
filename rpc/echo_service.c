#include "echo_service.h"

RPC(single_string_param, echo, 0, input, 0)
RPC(method, echo)
{
	StringBuffer_printf(&request->response_buffer,
			    JSONRPC_RESPONSE_STRING, request->id,
			    RPC(get_param, echo, input));
}

RPC(single_double_param, echo_double, 0, input, 0)
RPC(method, echo_double)
{
	StringBuffer_printf(&request->response_buffer,
			    JSONRPC_RESPONSE_DOUBLE, request->id,
			    RPC(get_param, echo_double, input));
}

RPC(single_longlong_param, echo_longlong, 0, input, 0)
RPC(method, echo_longlong)
{
	StringBuffer_printf(&request->response_buffer,
			    JSONRPC_RESPONSE_LONGLONG, request->id,
			    RPC(get_param, echo_longlong, input));
}

/* export RPC_Method definitions */
RPC(export, echo);
RPC(export, echo_double);
RPC(export, echo_longlong);
