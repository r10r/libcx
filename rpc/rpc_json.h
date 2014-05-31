#ifndef CX_RPC_JSON_H
#define CX_RPC_JSON_H

#include "rpc.h"

#define JSONRPC_VERSION "2.0"

#define JSONRPC_RESERVED_METHOD_PREFIX "rpc."
#define JSONRPC_RESERVED_METHOD_PREFIX_LEN 4

#define JSONRPC_INTERNAL_ERROR \
	"{\"jsonrpc\": \"2.0\", \"error\": {\"code\": -32603, \"message\": \"Internal error\"}, \"id\": null}"

/* @return -1 on error, else number of parameters */
int
Params_from_json(RPC_Param** params, json_t* json);

/* -1 on error (error code see request->error), 0 else*/
int
Request_from_json(RPC_Request* request, json_t* root);

json_t*
Value_to_json(RPC_Value* value);

json_t*
Request_create_json_response(RPC_Request* request);

json_t*
RPC_process(RPC_MethodTable* rpc_methods, Request* request);


#endif
