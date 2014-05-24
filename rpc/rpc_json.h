#ifndef CX_RPC_JSON_H
#define CX_RPC_JSON_H

#include "rpc.h"

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

#endif
