#ifndef CX_RPC_SERVICE_H
#define CX_RPC_SERVICE_H

#include <limits.h>
#include <stdbool.h>
#include "base/errno.h"
#include <string.h> /* strcmp */
#include "base/base.h"

#define JSON_RPC_ERROR_MIN      -32768
#define JSON_RPC_ERROR_MAX      -32000

typedef enum cx_json_rpc_error_t
{
	CX_RPC_ERROR_OK = CX_ERR_OK,
	CX_RPC_ERROR_PARSE = -32700,
	CX_RPC_ERROR_INVALID_REQUEST = -32600,
	CX_RPC_ERROR_METHOD_NOT_FOUND = -32601,
	CX_RPC_ERROR_INVALID_PARAMS = -32602,
	CX_RPC_ERROR_INTERNAL = -32603,
	/* -32000 to -32099 Server error, implementation defined */
} RPC_Error;


typedef struct json_t json_t;

/* parameter encoding format */
typedef enum
{
	FORMAT_NATIVE = 0,
	FORMAT_JSON
} RPC_Format;

typedef enum
{
	RPC_TYPE_VOID,
	RPC_TYPE_NULL,
	RPC_TYPE_INTEGER,
	RPC_TYPE_LONGLONG,
	RPC_TYPE_DOUBLE,
	RPC_TYPE_STRING,
	RPC_TYPE_BOOLEAN,
	RPC_TYPE_OBJECT,        // FIXME rename to RPC_TYPE_COMPLEX ?
} RPC_Type;

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
	RPC_ID_INVALID = 0,
	RPC_ID_NONE,
	RPC_ID_NUMBER,
	RPC_ID_STRING,
} RPC_ID_Type;

#define RPC_ERROR_MESSAGE_LENGTH_MAX 128

struct cx_rpc_request_t
{
	RPC_Error error;
	char* error_reason;     /* preallocate the error message buffer ? */

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

void
RPC_Request_set_error(RPC_Request* request, int err, const char* message);

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

const char*
cx_rpc_strerror(int err);

#endif
