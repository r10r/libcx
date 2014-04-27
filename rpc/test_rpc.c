#include "base/test.h"
#include "rpc.h"

/* namespace  declaration */
#undef RPC
#define RPC(action, ...) RPC_ ## action(Foobar, __VA_ARGS__)


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
RPC(set_param, foobar, 0, foo, const char*, get_foo_value, RPC_String, 0)
RPC(set_param, foobar, 1, bar, int, get_bar_value, RPC_LongLong, 0)

static void
test_parameter_definition()
{
	/* must be defined for RPC_get_param macro */
	RPC_Request* request = NULL;

	/* get by name */
	const char* foo = RPC(get_param, foobar, foo);
	int bar = RPC(get_param, foobar, bar);

	TEST_ASSERT_EQUAL(0, strcmp("foo", foo));
	TEST_ASSERT_EQUAL(666, bar);
}

// all parameters (only per macro)
RPC(param_list, foobar)
{
	&RPC(param, foobar, foo),
	&RPC(param, foobar, bar)
};

static void
test_parameter_list()
{
	// TODO implement
}

int
main()
{
	TEST_BEGIN

	RUN(test_parameter_definition);
	RUN(test_parameter_list);

	TEST_END
}
