#include "base/test.h"

#define RPC_NS MyNamespace_
#include "rpc.h"

// protocol api implementation

void
RPC_RequestList_free_data(RPC_RequestList* request_list)
{
}

/* deserialize methods */
static const char*
get_foo_value(RPC_Request* request, RPC_Param* param)
{
	return "foo";
}

static int
get_bar_value(RPC_Request* request, RPC_Param* param)
{
	return 666;
}

/* typesafe parameter definition */
RPC_set_param(foobar, 0, foo, string, get_foo_value, 0)
RPC_set_param(foobar, 1, bar, longlong, get_bar_value, 0)

static void
test_parameter_definition()
{
	/* must be defined for RPC_get_param macro */
	RPC_Request* request = cx_alloc(sizeof(RPC_Request));

	request->params = cx_alloc(2 * sizeof(RPC_Value));

	/* trigger deserialization */
	RPC_param_deserialize(foobar, foo) (request);
	RPC_param_deserialize(foobar, bar) (request);

	/* retrieve deserialized parameters */
	const char* foo_value = RPC_get_param(foobar, foo);
	long long bar_value = RPC_get_param(foobar, bar);

	TEST_ASSERT_EQUAL(0, strcmp("foo", foo_value));
	TEST_ASSERT_EQUAL(666, bar_value);

	cx_free(request->params);
	cx_free(request);
}

int
main()
{
	TEST_BEGIN

	RUN(test_parameter_definition);

	TEST_END
}
