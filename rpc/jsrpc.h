#ifndef _CX_JSRPC_H
#define _CX_JSRPC_H

#include <stdlib.h>
#include <stdio.h>
#include "list/list.h"
// dispatching non-positional parameters is difficult
// a simple wrapper that wrapps json RPC calls to method calls ?
// with optional validation

typedef enum json_type_t
{
	JS_OBJECT,
	JS_ARRAY,
	JS_STRING,
	JS_NUMBER,
	JS_BOOL
} JSON_Type;


typedef enum json_error_code
{
	jsrpc_ERROR_MALFORMED_JSON = -32700,
	jsrpc_ERROR_INVALID_REQUEST = -32600,
	jsrpc_ERROR_METHOD_NOT_FOUND = -32601,
	jsrpc_ERROR_INVALID_PARAMS = -32602,
	jsrpc_ERROR_INTERNAL = -32603,
	/* -32000 to -32099 Server error, implementation defined */
} JSON_ErrorCode;

typedef struct json_struct_t
{
	JSON_Type type;
	char* value; /* pointer to the value or integer value */
} JSON_Struct;

typedef struct json_value_t
{
	const char* name;
	JSON_Struct value;
} JSON_Value;

typedef struct rpc_error_t
{
	int code;
	const char* message;
	JSON_Struct* data;      /* additional error information */
} RPC_Error;

// load parameter data into param
// if name is null use index
// @return -1 on error, 0 if value does not exist, 1 on success
//typedef int jsrpc_get_param (RPC_Request* request, const char* name, int index, void* ptr);
//
//typedef int jsrpc_get_param_int (RPC_Request* request, const char* name, int index, int* ptr);
//typedef int jsrpc_get_param_double (RPC_Request* request, const char* name, int index, double* ptr);
//
//typedef int jsrpc_get_param (RPC_Request* request, const char* name, int index, void* ptr);

#define MAX_PARAMS 128

typedef struct rpc_request_t
{
	const char* method;
	const char* id;

	char* params[MAX_PARAMS];
} RPC_Request;

//typedef struct rpc_response_t
//{
//	const char* jsonrpc;
//	const char* id;
//	RPC_Error* error;
//	JSON_Struct* value;
//} RPC_Response;


enum internal_error_t
{
	DIVISION_BY_NULL,
};

// FIXME how to declare error codes
static const char* division_by_null = "Division by null";


/* TODO make it threadsafe */
static int rpc_errno;

// FIXME uppercase is difficult to read
#define JSONRPC_RESPONSE_HEADER "{\"jsonrpc\":\"2.0\",\"id\":%s,"
#define JSONRPC_RESULT_SIMPLE "\"result\":%s}\n"
#define JSONRPC_RESULT_STRING "\"result\":\"%s\"}\n"
#define JSONRPC_ERROR_SIMPLE "\"error\":{\"code\":%d,\"message\":\"%s\"}}\n"

static const char* const JSONRPC_RESPONSE = JSONRPC_RESPONSE_HEADER JSONRPC_RESULT_SIMPLE;
static const char* const JSONRPC_RESPONSE_STRING = JSONRPC_RESPONSE_HEADER JSONRPC_RESULT_STRING;
static const char* const JSONRPC_ERROR = JSONRPC_RESPONSE_HEADER JSONRPC_ERROR_SIMPLE;

static const char* JSONRPC_NULL = "null"; /* for invalid ID or null values*/

/*
 *  ssize_t
     write(int fildes, const void *buf, size_t nbyte);

    printf
 */

// write function ?
#define jsrpc_fprintf_response(fd, id, result) \
	fprintf(fd, JSONRPC_RESPONSE, id, result)

#define jsrpc_fprintf_error(fd, id, code, message) \
	fprintf(fd, JSONRPC_ERROR, (id ? id : JSONRPC_NULL), code, message)


/* parameter handling */

// TODO handle parameter validation (greater than, lower then, not null, ....)
// handle validation transparently in the deserialize callback ?
// or attach validator to param ?
// some small parser would be nice when using validation strings

#define ARRAY_SIZE( array ) sizeof( array ) / sizeof( array[0] )


// param values stored in an array
typedef struct jsrpc_param_t jsrpc_Param;
struct jsrpc_param_t
{
	const char* name;
	// union value ?
	unsigned int flags;
	void* (* deserialize) (jsrpc_Param* param, int index, void* data);
};

// http://stackoverflow.com/questions/2641473/initialize-static-array-of-structs-in-c

#define METHOD_SIGNATURE(name) \
	static jsrpc_Param METHOD_SIGNATURE_ ## name [] =

#define METHOD_SIGNATURE_get(name) \
	METHOD_SIGNATURE_ ## name


typedef void F_RPCMethod (RPC_Request* request);

static void
register_method(const char* method, F_RPCMethod* fmethod, jsrpc_Param signature[], int size);

#define REGISTER_METHOD(meth) \
	register_method(#meth, meth, METHOD_SIGNATURE_get(meth), ARRAY_SIZE(METHOD_SIGNATURE_get(meth)))


static void*
RPC_Request_get_param(RPC_Request* request, const char* name);

typedef struct rpc_method_t
{
	char* name;
	F_RPCMethod* method;
	jsrpc_Param* signature;
	int param_count;
} RPC_Method;

/* plugin API */

extern void*
get_param_value_string(jsrpc_Param* param, int index, void* data);

extern void
dispatch_request(List* methods, const char* json);

#endif
