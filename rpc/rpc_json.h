#ifndef CX_RPC_JSON_H
#define CX_RPC_JSON_H

#include "rpc.h"

#define JSONRPC_VERSION "2.0"

#define JSONRPC_RESERVED_METHOD_PREFIX "rpc."
#define JSONRPC_RESERVED_METHOD_PREFIX_LEN 4

typedef enum cx_json_rpc_error_t
{
	JSON_RPC_ERROR_PARSE_ERROR = -32700,
	JSON_RPC_ERROR_INVALID_REQUEST = -32600,
	JSON_RPC_ERROR_METHOD_NOT_FOUND = -32601,
	JSON_RPC_ERROR_INVALID_PARAMS = -32602,
	JSON_RPC_ERROR_INTERNAL = -32603,
	/* -32000 to -32099 Server error, implementation defined */
} JSON_RPC_Error;

/* @return -1 on error, else number of parameters */
int
Params_from_json(RPC_Param** params, json_t* json);

/*
 * -1 on error (error code see request->error), 0 else
 */
int
Request_json_parse(RPC_Request* request, const char* data, size_t data_len);

/* -1 on error (error code see request->error), 0 else*/
int
Request_from_json(RPC_Request* request, json_t* root);

json_t*
Value_to_json(RPC_Value* value);

json_t*
Request_create_json_response(RPC_Request* request);

void
RPC_Request_json_free(RPC_Request* request);

#endif
