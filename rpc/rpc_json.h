#ifndef CX_RPC_JSON_H
#define CX_RPC_JSON_H

#include "rpc.h"

typedef struct json_t json_t;

void
Params_from_json(Param** params, json_t* json);

#endif
