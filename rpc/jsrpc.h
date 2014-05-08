#ifndef _CX_JS_RPC_H
#define _CX_JS_RPC_H

#include "rpc.h"

// FIXME uppercase names are difficult to read

#define JSONRPC_VERSION "{\"jsonrpc\":\"2.0\""
#define JSONRPC_HEADER JSONRPC_VERSION ",\"id\":%s,"


/* [ simple responses ]*/
#define JSONRPC_RESULT_SIMPLE "\"result\":%s}"
#define JSONRPC_RESULT_STRING "\"result\":\"%s\"}"
#define JSONRPC_RESULT_DOUBLE "\"result\":%lf}"
#define JSONRPC_RESULT_LONGLONG "\"result\":%lld}"

#define JSONRPC_RESULT_TRUE "\"result\":true}"
#define JSONRPC_RESULT_FALSE "\"result\":false}"
#define JSONRPC_RESULT_NULL "\"result\":null}"

static const char* const JSONRPC_RESPONSE_SIMPLE = JSONRPC_HEADER JSONRPC_RESULT_SIMPLE;
static const char* const JSONRPC_RESPONSE_STRING = JSONRPC_HEADER JSONRPC_RESULT_STRING;
static const char* const JSONRPC_RESPONSE_DOUBLE = JSONRPC_HEADER JSONRPC_RESULT_DOUBLE;
static const char* const JSONRPC_RESPONSE_LONGLONG = JSONRPC_HEADER JSONRPC_RESULT_LONGLONG;
static const char* const JSONRPC_RESPONSE_TRUE = JSONRPC_HEADER JSONRPC_RESULT_TRUE;
static const char* const JSONRPC_RESPONSE_FALSE = JSONRPC_HEADER JSONRPC_RESULT_FALSE;
static const char* const JSONRPC_RESPONSE_NULL = JSONRPC_HEADER JSONRPC_RESULT_NULL;

#define JSONRPC_BOOLEAN(b) ((b == 0) ? "false" : "true")
#define JSONRPC_RESPONSE_BOOLEAN(b) ((b == 0) ? JSONRPC_RESPONSE_FALSE : JSONRPC_RESPONSE_TRUE)
#define JSONRPC_RESULT_BOOLEAN(b) ((b == 0) ? JSONRPC_RESULT_FALSE : JSONRPC_RESULT_TRUE)

/* [ errors ] */

#define JSONRPC_ERROR "\"error\":{\"code\":%d,\"message\":\"%s\"}}"
static const char* const JSONRPC_RESULT_ERROR = JSONRPC_ERROR;
static const char* const JSONRPC_RESPONSE_ERROR = JSONRPC_HEADER JSONRPC_ERROR;
static const char* JSONRPC_NULL = "null"; /* for invalid ID or null values*/


/* [ complex responses ] */

#define JSONRPC_RESULT "\"result\":%s"
#define JSONRPC_RESPONSE JSONRPC_HEADER JSONRPC_RESULT

#define JSONRPC_STRING "\"%s\""
#define JSRPC_KEYPAIR(k, v) "\"" k "\":" v
#define JSRPC_STRINGPAIR(k) JSRPC_KEYPAIR(k, JSONRPC_STRING)
#define JSRPC_OBJECT(o) "%s{" o "}"

#define JSONRPC_RESULT_OBJECT_START "{"
#define JSONRPC_RESULT_OBJECT_END "}}"

#define JSONRPC_RESULT_ARRAY_START "["
#define JSONRPC_RESULT_ARRAY_END "]}"

#define jsrpc_write(format, ...) \
	StringBuffer_aprintf(result_buffer, format, __VA_ARGS__)

#define jsrpc_write_append(format, ...) \
	StringBuffer_aprintf(result_buffer, format, __VA_ARGS__)

#define jsrpc_write_append_simple(chars) \
	StringBuffer_cat(result_buffer, chars)

#define jsrpc_write_error(code, message) \
	if (IS_RPC_REQUEST(request)) jsrpc_write(JSONRPC_ERROR_SIMPLE, (request)->id, code, message)

#define jsrpc_write_simple_response(format) \
	if (IS_RPC_REQUEST(request)) jsrpc_write(format, (request)->id)

#define jsrpc_write_response(format, ...) \
	if (IS_RPC_REQUEST(request)) jsrpc_write(format, (request)->id, __VA_ARGS__)


/* [ simple requests ] */

static const char* const JSONRPC_REQUEST_SIMPLE =
	JSONRPC_HEADER "\"method\":\"%s\"}";

static const char* const JSONRPC_REQUEST =
	JSONRPC_HEADER "\"method\":\"%s\",\"params\":%s}";

static const char* const JSONRPC_NOTIFICATION =
	JSONRPC_VERSION ",\"method\":\"%s\",\"params\":%s}";


/* [ error codes ] */

typedef enum cx_json_error_code
{
	jsrpc_ERROR_PARSE_ERROR = -32700,
	jsrpc_ERROR_INVALID_REQUEST = -32600,
	jsrpc_ERROR_METHOD_NOT_FOUND = -32601,
	jsrpc_ERROR_INVALID_PARAMS = -32602,
	jsrpc_ERROR_INTERNAL = -32603,
	/* -32000 to -32099 Server error, implementation defined */
} JSON_ErrorCode;

const char*
jsrpc_strerror(int code);

/* [ debug macros ] */

#define jsrpc_fprintf_response(fd, id, result) \
	fprintf(fd, JSONRPC_RESPONSE, id, result)

#define jsrpc_fprintf_error(fd, id, code, message) \
	fprintf(fd, JSONRPC_ERROR, (id ? id : JSONRPC_NULL), code, message)

#endif
