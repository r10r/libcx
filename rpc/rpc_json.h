#ifndef CX_RPC_JSON_H
#define CX_RPC_JSON_H

#include "rpc.h"

/* @return -1 on error, else number of parameters */
int
Params_from_json(RPC_Param** params, json_t* json);

json_t*
Value_to_json(RPC_Value* value);

#endif
