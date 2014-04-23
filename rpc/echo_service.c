#include "echo_service.h"

RPC(set_param, echo, 0, input, const char*, RPC_Request_get_param_string_value, RPC_String, 0)
RPC(param_list, echo)
{
	&RPC(param, echo, input)
};
RPC(method, echo)
{
	StringBuffer_printf(&request->response_buffer, JSONRPC_RESPONSE_STRING, request->id, RPC(get_param, echo, input));
}

/* export RPC_Method definitions */
RPC(export, echo);
