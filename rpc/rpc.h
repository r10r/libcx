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
	FORMAT_NATIVE = 0,
	FORMAT_JSON
} ParamFormat;

typedef enum
{
	ERROR_PARAM_NULL = 1,           /* a parameter that should not be null is null (precondition check in service method) */
	ERROR_NO_PARAMS,                /* no params are available (params are NULL) */
	ERROR_PARAM_MISSING,            /* required parameter is missing */
	ERROR_PARAM_INVALID_TYPE,       /* parameter has another type than specified by method signature */
	ERROR_PARAM_UNSUPPORTED_FORMAT, /* specified format unsupported (e.g no JSON deserializer) */
	ERROR_METHOD_MISSING,           /* service does not provide given method */
	ERROR_PARAM_DESERIALIZE,        /* a complex type can not be deserialized */
	ERROR_RESULT_VALUE_NULL         /* method has result but no result value given (internal error) */
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

// FIXME prefix every type with RPC_ ?

typedef struct cx_rpc_value_t Value;
typedef struct cx_rpc_param_t Param;
typedef struct cx_rpc_method_table_t MethodTable;
typedef struct cx_rpc_request_t RPC_Request;

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

struct cx_rpc_method_table_t
{
	const char* method_name;
	/* extracts/validates parameters, calls method, creates result value */
	RPC_MethodWrapper* method_wrapper;
};

struct cx_rpc_request_t
{
	ValueType id_type;
	union
	{
		long long longlong;
		char* string;
	} id;

	char* method_name;

	Param* params;
	int num_params;
	ParamFormat format;
	Value result;

	void* data; /* hold deserialized JSON ? */

	// TODO add f_free method
};

Value*
Param_get(Param* params, int position, const char* name, int num_params);

int
Service_call(MethodTable* method_map, RPC_Request* request);

#endif
