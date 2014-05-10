#include "base/test.h"
#include "rpc.h"

/* namespace  declaration */
#undef RPC
#define RPC(action, ...) RPC_ ## action(Foobar, __VA_ARGS__)

/* deserialize methods */
static const char*
get_foo_value(RPC_Param* param, void* data)
{
	return "foo";
}

static int
get_bar_value(RPC_Param* param, void* data)
{
	return 666;
}

/* typesafe parameter definition */
RPC(set_param, foobar, 0, foo, const char*, get_foo_value, RPC_Number, 0)
RPC(set_param, foobar, 1, bar, int, get_bar_value, RPC_Number, 0)

static void
test_parameter_definition()
{
	const char* foo;
	int bar;

	/* get by name */
	foo = RPC(get_param, foobar, foo);
	bar = RPC(get_param, foobar, bar);

	TEST_ASSERT_EQUAL(0, strcmp("foo", foo));
	TEST_ASSERT_EQUAL(666, bar);

	/* get by position */
	foo = RPC(get_param, foobar, 0);
	bar = RPC(get_param, foobar, 1);

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
