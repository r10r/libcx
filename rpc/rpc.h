#ifndef _CX_RPC_H
#define _CX_RPC_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>                     /* strdup */
#include "string/string_buffer.h"       /* response buffer */
#include "base/debug.h"

#define MAX_PARAMS 32                   /* maximum number of parameters */

typedef struct rpc_param_t RPC_Param;
typedef struct rpc_method_t RPC_Method;
typedef struct rpc_request_t RPC_Request;
typedef struct rpc_request_list_t RPC_RequestList;
typedef void F_RPC_Method (RPC_Request* request, StringBuffer* result_buffer);

typedef enum rpc_type_t
{
	RPC_Double,
	RPC_LongLong,
	RPC_String,
	RPC_Boolean,
	RPC_Array,
	RPC_Object,
	RPC_Null,
	RPC_Undefined
} RPC_Type;

struct rpc_param_t
{
	const char* name;
	RPC_Type type;
	unsigned int flags;
	int pos;
};

struct rpc_method_t
{
	const char* name;
	F_RPC_Method* method;
	RPC_Param** signature;
	int param_count;
};

struct rpc_request_t
{
	const char* id;
	const char* method_name;
	int error;
	RPC_Param* params[MAX_PARAMS];
	void* data;
	int response_written;
};

struct rpc_request_list_t
{
	StringBuffer* request_buffer;
	StringBuffer* response_buffer;
	StringBuffer* result_buffer; /* holds either result or error message */

	int nrequests;
	RPC_Request* requests;

	void* data;
};

#define ARRAY_SIZE( array ) \
	sizeof( array ) / sizeof( array[0] )

/* [ Utility macros */

#define IS_RPC_REQUEST(request) \
	(request->id != NULL)

/* [ RPC actions ] */

#define RPC_public_name(ns, meth) \
	ns ## _ ## meth

#define RPC_method_name(ns, meth) \
	ns ## _ ## meth ## _method

#define RPC_params_name(ns, meth) \
	ns ## _ ## meth ## _params


/* [ Method definition ] */

#define RPC_method(ns, meth) \
	void ns ## _ ## meth ## _method(RPC_Request * request, StringBuffer * result_buffer)

#define RPC_params(ns, meth) \
	static RPC_Param ns ## _ ## meth ## _params [] =

#define RPC_def(ns, meth) \
	{ #meth, RPC_method_name(ns, meth), RPC_params_name(ns, meth), ARRAY_SIZE(RPC_params_name(ns, meth)) }


/* [ Parameter Handling ] */

#define RPC_param(ns, meth, name) \
	ns ## _ ## meth ## _param_ ## name

#define RPC_param_deserialize(ns, meth, name) \
	RPC_param(ns, meth, name ## _deserialize)

#define RPC_param_define(ns, meth, _name, _rpc_type, _pos, _flags) \
	static RPC_Param RPC_param(ns, meth, _name) = \
	{ .name = #_name, .pos = _pos, .type = _rpc_type, .flags = _flags };

#define RPC_param_define_deserialize(ns, meth, name, param_name, type, func) \
	static inline \
	type RPC_param_deserialize(ns, meth, name) (RPC_Request * request) \
	{ return func(request, &RPC_param(ns, meth, param_name)); }

#define RPC_set_param(ns, meth, pos, name, type, func, rpc_type, flags) \
	RPC_param_define(ns, meth, name, rpc_type, pos, flags) \
	RPC_param_define_deserialize(ns, meth, name, name, type, func)

#define RPC_get_param(ns, meth, name) \
	RPC_param_deserialize(ns, meth, name) (request)

#define RPC_param_list(ns, meth) \
	static RPC_Param *  ns ## _ ## meth ## _params[] =


/* [ Convenience Macros ] */

#define RPC_set_param_string(ns, meth, pos, name, flags) \
	RPC_set_param(ns, meth, pos, name, const char*, RPC_Request_get_param_value_string, RPC_String, flags)

#define RPC_set_param_double(ns, meth, pos, name, flags) \
	RPC_set_param(ns, meth, pos, name, double, RPC_Request_get_param_value_double, RPC_Double, flags)

#define RPC_set_param_longlong(ns, meth, pos, name, flags) \
	RPC_set_param(ns, meth, pos, name, long long, RPC_Request_get_param_value_longlong, RPC_LongLong, flags)

#define RPC_param_list_single(ns, meth, name) \
	RPC_param_list(ns, meth) { &RPC_param(ns, meth, name) };

#define RPC_single_string_param(ns, meth, pos, name, flags) \
	RPC_set_param_string(ns, meth, pos, name, flags) \
	RPC_param_list_single(ns, meth, name)

#define RPC_single_double_param(ns, meth, pos, name, flags) \
	RPC_set_param_double(ns, meth, pos, name, flags) \
	RPC_param_list_single(ns, meth, name)

#define RPC_single_longlong_param(ns, meth, pos, name, flags) \
	RPC_set_param_longlong(ns, meth, pos, name, flags) \
	RPC_param_list_single(ns, meth, name)

/* [ Method Export ] */

#define RPC_export(ns, meth) \
	RPC_method(ns, meth); \
	static RPC_Method ns ## _ ## meth = RPC_def(ns, meth);

#define RPC_export_without_params(ns, meth) \
	RPC_method(ns, meth); \
	static RPC_Method ns ## _ ## meth = { #meth, RPC_method_name(ns, meth), NULL, 0 };

#define RPC_methods(ns) \
	ns ## _ ## methods


/* [RPC API] */

extern const RPC_Method RPC_Method_none;

void
RPC_Method_log(RPC_Method* method);

RPC_Request*
RPC_Request_new(void);

void
RPC_Request_free(RPC_Request* request);

RPC_Method*
RPC_Request_lookup_method(RPC_Request* request, RPC_Method methods[]);

RPC_RequestList*
RPC_RequestList_new(void);

void
RPC_RequestList_free(RPC_RequestList* request_list);


/* [ Protocol API ] */

void
RPC_RequestList_process(RPC_RequestList* request_list, RPC_Method methods[]);


/* [Protocol Plugin API] */

extern void
RPC_RequestList_free_data(RPC_RequestList* request_list);

/*
 * @return the number of requests (> 0)
 * -1 if input is invalid JSON
 * 0 if input is not a valid JSON RPC request object
 * @malloc request (count * sizeof(Request))
 */
extern int
RPC_Request_deserialize(RPC_RequestList* request_list);

extern const char*
RPC_Request_get_param_value_string(RPC_Request* request, RPC_Param* param);

extern long long
RPC_Request_get_param_value_longlong(RPC_Request* request, RPC_Param* param);

extern double
RPC_Request_get_param_value_double(RPC_Request* request, RPC_Param* param);

#endif
