#include "base/test.h"
#include "echo_service.h"
#include <jansson.h>

static RPC_Method*
select_method(RPC_Method methods[], const char* method_name)
{
	RPC_Method* meth = NULL;

	while ((meth = methods++) != NULL)
	{
		if (strcmp(meth->name, method_name) == 0)
			return meth;
	}
	return NULL;
}

static void
test_select_method()
{
	RPC_Method methods[] = { EchoService_methods, RPC_Method_none };
	RPC_Method* meth;

	meth = select_method(methods, "echo");
	TEST_ASSERT_EQUAL_STRING(meth->name, "echo");
	TEST_ASSERT_EQUAL_PTR(meth->method, EchoService_echo_method);

	meth = select_method(methods, "echo_double");
	TEST_ASSERT_EQUAL_STRING(meth->name, "echo_double");
	TEST_ASSERT_EQUAL_PTR(meth->method, EchoService_echo_double_method);

	meth = select_method(methods, "echo_longlong");
	TEST_ASSERT_EQUAL_STRING(meth->name, "echo_longlong");
	TEST_ASSERT_EQUAL_PTR(meth->method, EchoService_echo_longlong_method);
}

static void
test_call_method_echo()
{
	RPC_Method methods[] = { EchoService_methods, RPC_Method_none };
	RPC_Method* meth = select_method(methods, "echo");

	RPC_Value* param_values = cx_alloc(sizeof(RPC_Param) * 2);

	param_values[0].string_value = "hello world";

	Response* response = meth->method(param_values);

	const char* data = NULL;
	response->f_data_get(response, &data);

	TEST_ASSERT_EQUAL_STRING("\"hello world\"", data);

	cx_free(param_values);
	Response_free(response);
}

static void
test_call_method_echo_longlong()
{
	RPC_Method methods[] = { EchoService_methods, RPC_Method_none };
	RPC_Method* meth = select_method(methods, "echo_longlong");

	RPC_Value* param_values = cx_alloc(sizeof(RPC_Param) * 2);

	param_values[0].longlong_value = 123456;

	Response* response = meth->method(param_values);

	const char* data = NULL;
	response->f_data_get(response, &data);

	TEST_ASSERT_EQUAL_STRING("123456", data);

	cx_free(param_values);
	Response_free(response);
}

static void
test_call_method_echo_double()
{
	RPC_Method methods[] = { EchoService_methods, RPC_Method_none };
	RPC_Method* meth = select_method(methods, "echo_double");

	RPC_Value* param_values = cx_alloc(sizeof(RPC_Param) * 2);

	param_values[0].double_value = 2.666;

	Response* response = meth->method(param_values);

	const char* data = NULL;
	response->f_data_get(response, &data);

	TEST_ASSERT_EQUAL_STRING("2.666", data);

	cx_free(param_values);
	Response_free(response);
}

int
main()
{
	TEST_BEGIN

	RUN(test_select_method);
	RUN(test_call_method_echo);
	RUN(test_call_method_echo_longlong);
	RUN(test_call_method_echo_double);

	TEST_END
}
