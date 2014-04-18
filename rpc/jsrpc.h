#ifndef _CX_JS_RPC_H
#define _CX_JS_RPC_H

#include "rpc.h"

// FIXME uppercase names are difficult to read
#define JSONRPC_RESPONSE_HEADER "{\"jsonrpc\":\"2.0\",\"id\":%s,"
#define JSONRPC_RESULT_SIMPLE "\"result\":%s}\n"
#define JSONRPC_RESULT_STRING "\"result\":\"%s\"}\n"
#define JSONRPC_ERROR_SIMPLE "\"error\":{\"code\":%d,\"message\":\"%s\"}}\n"

static const char* const JSONRPC_RESPONSE = JSONRPC_RESPONSE_HEADER JSONRPC_RESULT_SIMPLE;
static const char* const JSONRPC_RESPONSE_STRING = JSONRPC_RESPONSE_HEADER JSONRPC_RESULT_STRING;
static const char* const JSONRPC_ERROR = JSONRPC_RESPONSE_HEADER JSONRPC_ERROR_SIMPLE;
static const char* JSONRPC_NULL = "null"; /* for invalid ID or null values*/

static const char* const JSONRPC_REQUEST =
	JSONRPC_RESPONSE_HEADER "\"method\":\"%s\",\"params\":{\"%s\":\"%s\"}}";

static const char* const JSONRPC_REQUEST_POS =
	JSONRPC_RESPONSE_HEADER "\"method\":\"%s\",\"params\":[\"%s\"]}";

typedef enum json_error_code
{
	jsrpc_ERROR_MALFORMED_JSON = -32700,
	jsrpc_ERROR_INVALID_REQUEST = -32600,
	jsrpc_ERROR_METHOD_NOT_FOUND = -32601,
	jsrpc_ERROR_INVALID_PARAMS = -32602,
	jsrpc_ERROR_INTERNAL = -32603,
	/* -32000 to -32099 Server error, implementation defined */
} JSON_ErrorCode;

// write function ?
#define jsrpc_fprintf_response(fd, id, result) \
	fprintf(fd, JSONRPC_RESPONSE, id, result)

#define jsrpc_fprintf_error(fd, id, code, message) \
	fprintf(fd, JSONRPC_ERROR, (id ? id : JSONRPC_NULL), code, message)

#endif
