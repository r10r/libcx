#include "echo_service.h"

RPC(method, echo)
{
	StringBuffer_printf(&request->response_buffer,
			    JSONRPC_RESPONSE_STRING, request->id,
			    RPC(get_param, echo, input));
}


RPC(method, echo_double)
{
	StringBuffer_printf(&request->response_buffer,
			    JSONRPC_RESPONSE_DOUBLE, request->id,
			    RPC(get_param, echo_double, input));
}

RPC(method, echo_longlong)
{
	StringBuffer_printf(&request->response_buffer,
			    JSONRPC_RESPONSE_LONGLONG, request->id,
			    RPC(get_param, echo_longlong, input));
}
