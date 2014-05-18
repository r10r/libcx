#ifndef CX_RPC_SERVICE_H
#define CX_RPC_SERVICE_H

#include <limits.h>
#include <stdbool.h>
#include "base/errno.h"
#include <string.h> /* strcmp */
#include "base/base.h"

typedef struct json_t json_t;

typedef enum
{
	FORMAT_NATIVE,
	FORMAT_JSON
} ParamFormat;

typedef enum
{
	ERROR_PARAM_NULL = 1,
	ERROR_NO_PARAMS,
	ERROR_PARAM_MISSING,
	ERROR_PARAM_INVALID_TYPE,
	ERROR_PARAM_UNSUPPORTED_FORMAT,
	ERROR_METHOD_MISSING,
	ERROR_PARAM_DESERIALIZE,
	ERROR_RESULT_VALUE_NULL /* method has result but no result value given (internal error) */
} RPC_Error;


typedef enum
{
	TYPE_INTEGER,
	TYPE_LONGLONG,
	TYPE_DOUBLE,
	TYPE_STRING,
	TYPE_BOOLEAN,
	TYPE_NULL,
	TYPE_OBJECT,
} ValueType;


typedef struct cx_rpc_value_t Value;
typedef struct cx_rpc_param_t Param;
typedef struct cx_rpc_method_map_t MethodTable;

/*
 * @param object the param object of the union value
 */
typedef void F_ValueFree (void* object);

/*
 * @param deserialize function only set/used when type is object
 */
typedef json_t* F_ValueToJSON (void* object);
typedef int RPC_MethodWrapper (Param* params, int num_params, Value* result, ParamFormat format);


struct cx_rpc_value_t
{
	ValueType type;
	union
	{
		int integer;
		long long longlong;
		double floatingpoint;
		bool boolean;
		char* string;
		void* object;
	} value;

	F_ValueFree* f_free;
	F_ValueToJSON* f_to_json;
};

struct cx_rpc_param_t
{
	Value value;
	int position;
	const char* name;
};

struct cx_rpc_method_map_t
{
	const char* name;
	RPC_MethodWrapper* method;
};

Value*
Param_get(Param* params, int position, const char* name, int num_params);

/* TODO move parameters into Request struct */
int
Service_call(MethodTable* method_map, const char* method_name, Param* params, int num_params, Value* result, ParamFormat format);

#endif
