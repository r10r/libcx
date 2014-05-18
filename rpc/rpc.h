#ifndef CX_RPC_SERVICE_H
#define CX_RPC_SERVICE_H

#include <limits.h>
#include <stdbool.h>
#include "base/errno.h"
#include <string.h> /* strcmp */
#include "base/base.h"

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
	ERROR_PARAM_TYPE,
	ERROR_PARAM_FORMAT,
	ERROR_METHOD_MISSING,
	ERROR_RESULT_VALUE_NULL /* method has result but no result value given (internal error) */
} RPC_Error;


typedef enum
{
	TYPE_INTEGER,
	TYPE_LONGLONG,
	TYPE_DOUBLE,
	TYPE_STRING,
	TYPE_BOOLEAN,
	TYPE_OBJECT,
	TYPE_NULL,
} ValueType;


typedef struct cx_rpc_value_t Value;
typedef struct cx_rpc_param_t Param;
typedef struct cx_rpc_method_map_t MethodMap;

typedef void F_ValueFree (void* object);
typedef int RPC_Method (Param* params, int num_params, Value* result, ParamFormat format);


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
	}

	value;

	F_ValueFree* f_free;
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
	RPC_Method* method;
};

Value*
Param_get(Param* params, int position, const char* name, int num_params);

/* TODO move parameters into Request struct */
int
Service_call(MethodMap* method_map, const char* method_name, Param* params, int num_params, Value* result, ParamFormat format);

#endif
