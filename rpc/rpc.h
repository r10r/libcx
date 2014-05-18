#ifndef CX_RPC_SERVICE_H
#define CX_RPC_SERVICE_H

#include <limits.h>
#include <stdbool.h>
#include "base/errno.h"
#include <string.h> /* strcmp */
#include "base/base.h"

typedef struct value_t Value;
typedef struct param_t Param;
//typedef json_t* F_ValueToJSON (void* value);
typedef void F_ValueFree (void* value);

typedef enum
{
	FORMAT_NATIVE,
	FORMAT_JSON
} ParamFormat;

enum error
{
	ERROR_PARAM_NULL = 1,
	ERROR_NO_PARAMS,
	ERROR_PARAM_MISSING,
	ERROR_PARAM_TYPE,
	ERROR_METHOD_MISSING,
	ERROR_RESULT_VALUE_NULL /* method has result but no result value given (internal error) */
};

typedef enum
{
	TYPE_INTEGER,
	TYPE_LONGLONG,
	TYPE_DOUBLE,
	TYPE_STRING,
	TYPE_BOOLEAN,
	TYPE_OBJECT,
	TYPE_NULL,
} ValueType;

/* basic types | complex types */
// custom serialization/deserialization of objects (structs)

/* one serialization for simple types integer|floatingpoint|boolean|string|null */
/* custom serialization methods for object types */

struct value_t
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

//	F_ValueToJSON* f_to_json;  /* serialize object to e.g jansson */
	F_ValueFree* f_free;
};


struct param_t
{
	Value value;
	int position;
	const char* name;
};

Value*
Param_get(Param* params, int position, const char* name, int num_params);

#endif
