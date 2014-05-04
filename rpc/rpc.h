#ifndef _CX_RPC_H
#define _CX_RPC_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h> /* strdup */

#define MAX_PARAMS 32

typedef struct rpc_request_t
{
	const char* method;
	const char* id;

	char* params[MAX_PARAMS];
} RPC_Request;

typedef struct rpc_param_t RPC_Param;
typedef struct rpc_method_t RPC_Method;
typedef void F_RPC_Method (RPC_Request* request);

struct rpc_param_t
{
	const char* name;
	// union value ?
	void* (* deserialize) (RPC_Param* param, int index, void* data);
	unsigned int flags;
};

struct rpc_method_t
{
	const char* name;
	F_RPC_Method* method;
	RPC_Param* signature;
	int param_count;
};

#define ARRAY_SIZE( array ) \
	sizeof( array ) / sizeof( array[0] )

/* [RPC actions] */

#define RPC_public_name(ns, meth) \
	ns ## _ ## meth

#define RPC_method_name(ns, meth) \
	ns ## _ ## meth ## _method

#define RPC_params_name(ns, meth) \
	ns ## _ ## meth ## _params

/* ---------------------------- */

#define RPC_method(ns, meth) \
	void ns ## _ ## meth ## _method(RPC_Request * request)

#define RPC_params(ns, meth) \
	static RPC_Param ns ## _ ## meth ## _params [] =

#define RPC_def(ns, meth) \
	{ #meth, RPC_method_name(ns, meth), RPC_params_name(ns, meth), ARRAY_SIZE(RPC_params_name(ns, meth)) }

/* ---------------------------- */

#define RPC_export(ns, meth) \
	RPC_Method ns ## _ ## meth = RPC_def(ns, meth)

#define RPC_export_without_params(ns, meth) \
	RPC_Method ns ## _ ## meth = { #meth, RPC_method_name(ns, meth), NULL, 0 }

#define RPC_public(ns, meth) \
	RPC_method(ns, meth); \
	extern RPC_Method ns ## _ ## meth;

#define RPC_methods(ns) \
	ns ## _ ## methods

/* [Service API] */

extern const RPC_Method RPC_null;

RPC_Method*
RPC_Method_new(const char* name, F_RPC_Method* fmethod, RPC_Param signature[], int param_count);

void
RPC_Method_free(RPC_Method* method);

void
RPC_Method_log(RPC_Method* method);

extern void
register_method(const char* method, F_RPC_Method* fmethod, RPC_Param signature[], int size);

/* [Plugin API] */

extern void*
get_param_value_string(RPC_Param* param, int index, void* data);

extern void
dispatch_request(RPC_Method methods[], const char* json);

#endif
