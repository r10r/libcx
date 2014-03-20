#include "libcx-base/test.h"
#include "message.h"

NOSETUP

static void test_Message_new()
{
	Message *message = Message_new(1024);

	TEST_ASSERT_EQUAL_INT(1024, message->parser_state->buffer->length);
	TEST_ASSERT_EQUAL_INT(0, message->parser_state->buffer->string->length);
	TEST_ASSERT_EQUAL_INT(0, message->protocol_values->length);
	TEST_ASSERT_EQUAL_INT(0, message->headers->length);
	Message_free(message);
}

#define List_S(list, index) \
	((String*)List_get(list, index))

#define TEST_ASSERT_EQUAL_S(expected, s_actual) \
	TEST_ASSERT_EQUAL_MEMORY(expected, s_actual->value, s_actual->length);

static const char data[] =
	"VERIFY /foo/bar\n"
	"Header1: value1\n"
	"Header2: value2\n"
	"\n"
	"Hello World";

static void
test_assert_message(Message *message)
{
	TEST_ASSERT_EQUAL_S(data, message->parser_state->buffer->string);

	TEST_ASSERT_EQUAL_INT(2, message->protocol_values->length);
	TEST_ASSERT_EQUAL_S("VERIFY", List_S(message->protocol_values, 0));
	TEST_ASSERT_EQUAL_S("/foo/bar", List_S(message->protocol_values, 1));

	TEST_ASSERT_EQUAL_INT(2, message->headers->length);
	StringPair *header1 = (StringPair*)List_get(message->headers, 0);
	TEST_ASSERT_EQUAL_S("Header1",  header1->key);
	TEST_ASSERT_EQUAL_S("value1",  header1->value);

	StringPair *header2 = (StringPair*)List_get(message->headers, 1);
	TEST_ASSERT_EQUAL_S("Header2",  header2->key);
	TEST_ASSERT_EQUAL_S("value2",  header2->value);

	TEST_ASSERT_EQUAL_S("Hello World", message->body->string);
}

static void
test_Message_parse_single_pass()
{
	Message *message =  Message_new(1024);

	Message_buffer_append(message, data, strlen(data));
	Message_parse_finish(message);
	test_assert_message(message);
	Message_free(message);
}

static void test_Message_parse_multi_pass()
{
	Message *message = Message_new(0);
	unsigned int i;

	for (i = 0; i < strlen(data) - 1; i++)
	{
		Message_buffer_append(message, &data[i], 1);
		Message_parse(message);
	}
	// last run (i is already incremented by loop header)
	Message_buffer_append(message, &data[i], 1);
	Message_parse_finish(message);

	TEST_ASSERT_EQUAL_INT(strlen(data), message->parser_state->iterations);

	test_assert_message(message);
	Message_free(message);
}

int main()
{
	TEST_BEGIN

	RUN(test_Message_new);
	RUN(test_Message_parse_single_pass);
	RUN(test_Message_parse_multi_pass);

	TEST_END
}
