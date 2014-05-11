#include "base/test.h"

typedef enum rpc_param_type_t
{
	RPC_STRING
} RPC_Type;


typedef union rpc_param_value_t RPC_Value;
typedef struct rpc_param_t RPC_Param;
typedef struct rpc_request_t RPC_Request;
typedef void F_RPC_Param_deserialize (RPC_Request* request);

union rpc_param_value_t
{
	long long longlong_value;
	double double_value;
	const char* string_value;
	void* ptr_value;
};

struct rpc_param_t
{
	const char* name;
	RPC_Type type;
	unsigned int flags;
	int pos;
	F_RPC_Param_deserialize* f_deserialize;
};

struct rpc_request_t
{
	const char* id;
	const char* method_name;
	int error;
	void* data;
	int response_written;
	RPC_Value* params;
};

#define TYPE_STRING const char*
#define TYPE_STRING_ACCESSOR string_value

#define declare_param_deserialize(_name, _type) \
	static inline void param_deserialize_ ## _name(RPC_Request* request);

#define define_param_deserialize(_name, _type, _pos, _func) \
	static inline void param_deserialize_ ## _name(RPC_Request* request) \
	{       request->params[_pos].TYPE_ ## _type ## _ACCESSOR = _func(request, &param_ ## _name); }

#define define_param_get(_name, _type, _pos) \
	static inline TYPE_ ## _type param_get_ ## _name(RPC_Request* request) \
	{ return request->params[_pos].TYPE_ ## _type ## _ACCESSOR; }

#define define_param(_name, _pos, _type, _flags) \
	static RPC_Param param_ ## _name = \
	{ .name = #_name, .pos = _pos, .type = RPC_ ## _type, .flags = _flags, .f_deserialize = param_deserialize_ ## _name };


static const char*
deserialize_string(RPC_Request* request, RPC_Param* param)
{
	return (char*)(request->data);
}

#define method_param(_name, _type, _pos, _func, _flags) \
	declare_param_deserialize(_name, _type) \
	define_param(_name, _pos, _type, _flags) \
	define_param_deserialize(_name, _type, _pos, _func) \
	define_param_get(_name, _type, _pos)

#define param_get(_name) \
	param_get_ ## _name(&request)

/* use __func__ and __FILE__ as prefix to define params */


method_param(blubber, STRING, 0, deserialize_string, 0)
static void
test_params_union()
{
	RPC_Request request;

	request.data = (void*)"foobar";

	request.params = cx_alloc(sizeof(RPC_Value));

	param_deserialize_blubber(&request);

	TEST_ASSERT_EQUAL_STRING("foobar", param_get(blubber));

	cx_free(request.params);
}

int
main()
{
	TEST_BEGIN

	RUN(test_params_union);

	TEST_END
}
