#ifndef _CX_RPC_H
#define _CX_RPC_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>                     /* strdup */
#include "string/string_buffer.h"       /* response buffer */
#include "base/debug.h"
#include "list/list.h"

typedef union cx_rpc_param_value_t RPC_Value;
typedef struct cx_rpc_param_t RPC_Param;
typedef struct cx_rpc_method_t RPC_Method;
typedef struct cx_rpc_request_t RPC_Request;
typedef struct cx_rpc_request_list_t RPC_RequestList;
typedef void F_RPC_Method (RPC_Request* request, StringBuffer* result_buffer);
typedef void F_RPC_Param_deserialize (RPC_Request* request);

union cx_rpc_param_value_t
{
	int int_value;
	double double_value;
	long long longlong_value;
	const char* string_value;
	void* ptr_value;
};

#define RPC_TYPE_int int
#define RPC_TYPE_ACCESSOR_int int_value

#define RPC_TYPE_double double
#define RPC_TYPE_ACCESSOR_double double_value

#define RPC_TYPE_longlong long long
#define RPC_TYPE_ACCESSOR_longlong longlong_value

#define RPC_TYPE_string const char*
#define RPC_TYPE_ACCESSOR_string string_value

#define RPC_TYPE_boolean int
#define RPC_TYPE_ACCESSOR_boolean int_value

#define RPC_TYPE_array void*
#define RPC_TYPE_ACCESSOR_array ptr_value

#define RPC_TYPE_object void*
#define RPC_TYPE_ACCESSOR_array ptr_value

typedef enum cx_rpc_param_type_t
{
	RPC_double,
	RPC_longlong,
	RPC_string,
	RPC_boolean,
	RPC_array,
	RPC_object,
	RPC_undefined
} RPC_Type;

struct cx_rpc_param_t
{
	const char* name;
	RPC_Type type;
	unsigned int flags;
	int pos;
	F_RPC_Param_deserialize* f_deserialize;
};

struct cx_rpc_method_t
{
	const char* name;
	F_RPC_Method* method;
	RPC_Param** signature;
	int param_count;
};

struct cx_rpc_request_t
{
	const char* id;
	const char* method_name;
	int error;
	void* data;
	int response_written;
	RPC_Value* params;
};

struct cx_rpc_request_list_t
{
	StringBuffer* request_buffer;
	StringBuffer* response_buffer;
	StringBuffer* result_buffer; /* holds either result or error message */
	List* response_list;

	int nrequests;
	RPC_Request* requests;
	int batch;

	void* data;
};

#define ARRAY_SIZE( array ) \
	sizeof( array ) / sizeof( array[0] )

/* [ Utility macros */

#define IS_RPC_REQUEST(request) \
	(request->id != NULL)


/* [ RPC namespace expansion ] */

#ifndef RPC_NS
#define RPC_NS NamespaceUndefined_
#endif

#define RPC_ns_paste(prefix, token) prefix ## token
#define RPC_ns_expand(prefix, token) RPC_ns_paste(prefix, token)
#define RPC_ns(token) RPC_ns_expand(RPC_NS, token)


/* [ RPC actions ] */


#define RPC_public_name(meth) \
	RPC_ns(meth)

#define RPC_method_name(meth) \
	RPC_ns(meth ## _method)

#define RPC_params_name(meth) \
	RPC_ns(meth ## _params)

/* [ Method definition ] */

#define RPC_method(meth) \
	void RPC_ns(meth ## _method) (RPC_Request * request, StringBuffer * result_buffer)

#define RPC_params(meth) \
	static RPC_Param RPC_ns( ## meth ## _params)[] =

#define RPC_def(meth) \
	{ #meth, RPC_method_name(meth), RPC_params_name(meth), ARRAY_SIZE(RPC_params_name(meth)) }


/* [ Parameter Handling ] */

#define RPC_param(meth, name) \
	RPC_ns(meth ## _param_ ## name)

#define RPC_param_deserialize(meth, name) \
	RPC_param(meth, name ## _deserialize)

#define RPC_param_get(meth, name) \
	RPC_param(meth, name ## _get)

#define RPC_param_define(meth, _name, _type, _pos, _func, _flags) \
	static RPC_Param RPC_param(meth, _name) = \
	{ .name = #_name, .pos = _pos, .type = RPC_ ## _type, .flags = _flags, .f_deserialize = RPC_param_deserialize(meth, _name) };

#define RPC_param_declare_serialize(meth, name) \
	static inline void RPC_param_deserialize(meth, name) (RPC_Request * request);

#define RPC_param_define_deserialize(meth, name, _pos, _type, _func) \
	static inline void \
	RPC_param_deserialize(meth, name) (RPC_Request * request) \
	{ request->params[_pos].RPC_TYPE_ACCESSOR_ ## _type = _func(request, &RPC_param(meth, name)); }

#define RPC_param_define_get(meth, name, _type) \
	static inline RPC_TYPE_ ## _type \
	RPC_param_get(meth, name) (RPC_Request * request) \
	{ return request->params[RPC_param(meth, name).pos].RPC_TYPE_ACCESSOR_ ## _type; }

#define RPC_set_param(meth, pos, name, _type, _func, flags) \
	RPC_param_declare_serialize(meth, name) \
	RPC_param_define(meth, name, _type, pos, _func, flags) \
	RPC_param_define_deserialize(meth, name, pos, _type, _func) \
	RPC_param_define_get(meth, name, _type)

#define RPC_get_param(meth, name) \
	RPC_param_get(meth, name) (request)

#define RPC_param_list(meth) \
	static RPC_Param *  RPC_ns(meth ## _params)[] =


/* [ Convenience Macros ] */

#define RPC_set_param_string(meth, pos, name, flags) \
	RPC_set_param(meth, pos, name, string, RPC_Request_get_param_value_string, flags)

#define RPC_set_param_double(meth, pos, name, flags) \
	RPC_set_param(meth, pos, name, double, RPC_Request_get_param_value_double, flags)

#define RPC_set_param_longlong(meth, pos, name, flags) \
	RPC_set_param(meth, pos, name, longlong, RPC_Request_get_param_value_longlong, flags)

#define RPC_param_list_single(meth, name) \
	RPC_param_list(meth) { &RPC_param(meth, name) };

#define RPC_single_string_param(meth, pos, name, flags) \
	RPC_set_param_string(meth, pos, name, flags) \
	RPC_param_list_single(meth, name)

#define RPC_single_double_param(meth, pos, name, flags) \
	RPC_set_param_double(meth, pos, name, flags) \
	RPC_param_list_single(meth, name)

#define RPC_single_longlong_param(meth, pos, name, flags) \
	RPC_set_param_longlong(meth, pos, name, flags) \
	RPC_param_list_single(meth, name)

/* [ Method Export ] */

#define RPC_export(meth) \
	RPC_method(meth); \
	static RPC_Method RPC_ns(meth) = RPC_def(meth);

#define RPC_export_without_params(meth) \
	RPC_method(meth); \
	static RPC_Method RPC_ns(meth) = { #meth, RPC_method_name(meth), NULL, 0 };


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
RPC_RequestList_new(size_t buffer_size);

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
