#include "echo_service.h"

RPC(single_string_param, echo, 0, input, 0)
RPC(method, echo)
{
	StringBuffer_printf(&request->response_buffer, JSONRPC_RESPONSE_STRING, request->id, RPC(get_param, echo, input));
}

/* export RPC_Method definitions */
RPC(export, echo);
