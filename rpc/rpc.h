#ifndef CX_RPC_SERVICE_H
#define CX_RPC_SERVICE_H

#include <limits.h>
#include <stdbool.h>
#include <string.h> /* strcmp */

#include <libcx/base/errno.h>
#include <libcx/base/base.h>
#include <libcx/socket/request.h>

/* opaque JSON type declaration (implemented by jansson) */
typedef struct json_t json_t;

#define RPC_ERROR_MESSAGE_LENGTH_MAX 128

#define JSON_RPC_ERROR_MIN      -32768
#define JSON_RPC_ERROR_MAX      -32000

typedef enum cx_json_rpc_error_t
{
	CX_RPC_ERROR_OK = CX_ERR_OK,
	/* the following errors are always returned to the client */
	CX_RPC_ERROR_PARSE = -32700,
	CX_RPC_ERROR_INVALID_REQUEST = -32600,
	/* the following errors are only returned if the request is not a notification */
	CX_RPC_ERROR_METHOD_NOT_FOUND = -32601,
	CX_RPC_ERROR_INVALID_PARAMS = -32602,
	CX_RPC_ERROR_INTERNAL = -32603,
	/* -32000 to -32099 Server error, implementation defined */
} RPC_Error;

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
	RPC_TYPE_OBJECT
} RPC_Type;

typedef struct cx_rpc_value_t RPC_Value;
typedef struct cx_rpc_param_t RPC_Param;
typedef struct cx_rpc_method_table_t RPC_MethodTable;
typedef struct cx_rpc_request_t RPC_Request;
typedef struct cx_rpc_result_t RPC_Result;

/*
 * @param object the param object of the union value
 */
typedef void F_RPC_ValueFree (void* object);

/*
 * @param deserialize function only set/used when type is object
 */
typedef json_t* F_ValueToJSON (void* object);
typedef void RPC_MethodWrapper (RPC_Param* params, int num_params, RPC_Result* result, RPC_Format format);

struct cx_rpc_value_t
{
	RPC_Type type;
	union
	{
		int integer;
		long long longlong;
		double floatingpoint;
		bool boolean;
		const char* string;
		void* object;
	} data;

	F_RPC_ValueFree* f_free;
	F_ValueToJSON* f_to_json;
};

struct cx_rpc_param_t
{
	RPC_Value value;
	int position;
	const char* name;
};

/* result value */
struct cx_rpc_result_t
{
	RPC_Value value;
	RPC_Error error;
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

struct cx_rpc_request_t
{
	Request* request;
	RPC_ID_Type id_type;
	union
	{
		long long number;
		const char* string;
	} id;

	const char* method_name;

	RPC_Param* params;
	int num_params;
	RPC_Format format;
	RPC_Result result;
};

void
RPC_Result_set_error(RPC_Result* request, int err, const char* message);

#define RPC_Result_error(result, err_code) \
	RPC_Result_set_error(result, err_code, NULL)

#define RPC_Result_error_with_message(result, err_code, err_message) \
	RPC_Result_set_error(result, err_code, err_message)

RPC_Value*
Param_get(RPC_Param* params, int position, const char* name, int num_params);

void
Service_call(RPC_MethodTable* method_map, RPC_Request* request);

const char*
cx_rpc_strerror(RPC_Error err);

void
RPC_Request_init(RPC_Request* rpc_request, Request* request);
void
RPC_Request_free(RPC_Request* request);

void
cx_rpc_free_simple(void* object);

#endif
