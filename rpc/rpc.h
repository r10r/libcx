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
} RPC_Format;

/* FIXME cx_errno reserved for RPC calls ?, what about jsonrpc status codes ? */
typedef enum cx_rpc_error_t
{
	RPC_ERROR_OK = 0,                       /* no error */
	RPC_ERROR_REQUEST_PARSE,                /* failed to parse request format */
	RPC_ERROR_REQUEST_INVALID,              /* failed to unpack request */
	RPC_ERROR_FORMAT_UNSUPPORTED,           /* specified format not implemented in method (e.g no JSON deserializer) */
	RPC_ERROR_METHOD_MISSING,               /* service does not provide given method */
	RPC_ERROR_NO_PARAMS,                    /* method has params but no params are available (params are NULL) */
	RPC_ERROR_PARAM_MISSING,                /* a required parameter is missing */
	RPC_ERROR_PARAM_NULL,                   /* a required parameter has null value (precondition check in service method) */
	RPC_ERROR_PARAM_INVALID_TYPE,           /* a required parameter has another type than specified by method signature */
	RPC_ERROR_PARAM_INVALID_VALUE,          /* a required parameter has an invalid value */
	RPC_ERROR_PARAM_DESERIALIZE,            /* deserizliation of a parameter with RPC_TYPE_OBJECT failed */
	RPC_ERROR_RESULT_VALUE_NULL,            /* method has result but no result value given (internal error) */
} RPC_Error;

typedef enum
{
	RPC_TYPE_INTEGER,
	RPC_TYPE_LONGLONG,
	RPC_TYPE_DOUBLE,
	RPC_TYPE_STRING,
	RPC_TYPE_BOOLEAN,
	RPC_TYPE_NULL,
	RPC_TYPE_OBJECT,        // FIXME rename to RPC_TYPE_COMPLEX ?
} RPC_Type;

// FIXME prefix every type with RPC_ ?

typedef struct cx_rpc_value_t RPC_Value;
typedef struct cx_rpc_param_t RPC_Param;
typedef struct cx_rpc_method_table_t RPC_MethodTable;
typedef struct cx_rpc_request_t RPC_Request;

/*
 * @param object the param object of the union value
 */
typedef void F_RPC_ValueFree (void* object);
typedef void F_RPC_RequestFree (RPC_Request* request);

/*
 * @param deserialize function only set/used when type is object
 */
typedef json_t* F_ValueToJSON (void* object);
typedef int RPC_MethodWrapper (RPC_Param* params, int num_params, RPC_Value* result, RPC_Format format);


struct cx_rpc_value_t
{
	RPC_Type type;
	union
	{
		int integer;
		long long longlong;
		double floatingpoint;
		bool boolean;
		char* string;
		void* object;
	} value;

	F_RPC_ValueFree* f_free;
	F_ValueToJSON* f_to_json;
};

struct cx_rpc_param_t
{
	RPC_Value value;
	int position;
	const char* name;
};

struct cx_rpc_method_table_t
{
	const char* method_name;
	/* extracts/validates parameters, calls method, creates result value */
	RPC_MethodWrapper* method_wrapper;
};

typedef enum
{
	RPC_ID_NONE = 0,
	RPC_ID_NUMBER,
	RPC_ID_STRING,
} RPC_ID_Type;

struct cx_rpc_request_t
{
	RPC_Error error;
	RPC_ID_Type id_type;
	union
	{
		long long number;
		char* string;
	} id;

	char* method_name;

	RPC_Param* params;
	int num_params;
	RPC_Format format;
	RPC_Value result;

	void* data; /* contains deserialized JSON */
	F_RPC_RequestFree* f_free;

	// TODO add f_free method
};

RPC_Value*
Param_get(RPC_Param* params, int position, const char* name, int num_params);

/*
 * @return
 *     -1 on error (see request->error for error code)
 *      0 on success if method has void return type
 *      1 on success if method has non-void return type
 */
int
Service_call(RPC_MethodTable* method_map, RPC_Request* request);

#endif
