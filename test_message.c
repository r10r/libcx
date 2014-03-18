#include "libcx-base/test.h"
#include "message.h"

NOSETUP

static void test_Message_new()
{
	Message *message = Message_new(1024);

	TEST_ASSERT_EQUAL_INT(0, String_length(message->parser_state->buffer));
	TEST_ASSERT_EQUAL_INT(1024, String_available(message->parser_state->buffer));
	TEST_ASSERT_EQUAL_INT(0, message->protocol_values->length);
	TEST_ASSERT_EQUAL_INT(0, message->headers->length);
	Message_free(message);
}

static void test_Message_parse_single_pass()
{
	Message *message =  Message_new(1024);

	const char *data =
		"VERIFY /foo/bar\n"
		"Header1: value1\n"
		"Header2: value2\n"
		"\n"
		"Hello World";

	Message_buffer_append(message, data, (unsigned int)strlen(data));
	ParseEvent event = Message_parse_finish(message);

	TEST_ASSERT_EQUAL_STRING(data, message->parser_state->buffer);

	TEST_ASSERT_EQUAL_INT(2, message->protocol_values->length);
	TEST_ASSERT_EQUAL_STRING("VERIFY", List_get(message->protocol_values, 0));
	TEST_ASSERT_EQUAL_STRING("/foo/bar", List_get(message->protocol_values, 1));

	TEST_ASSERT_EQUAL_INT(2, message->headers->length);
	Pair *header1 = (Pair*)List_get(message->headers, 0);
	TEST_ASSERT_EQUAL_STRING("Header1", header1->key);
	TEST_ASSERT_EQUAL_STRING("value1", header1->value);

	Pair *header2 = (Pair*)List_get(message->headers, 1);
	TEST_ASSERT_EQUAL_STRING("Header2", header2->key);
	TEST_ASSERT_EQUAL_STRING("value2", header2->value);

	TEST_ASSERT_EQUAL_STRING("Hello World", message->body);

	Message_free(message);
}

static void test_Message_parse_multi_pass()
{
	Message *message = Message_new(1024);

	const char *data =
		"VERIFY /foo/bar\n"
		"Header1: value1\n"
		"Header2: value2\n"
		"\n"
		"Hello World";

	unsigned int i;

	for (i = 0; i < strlen(data); i++)
	{
		Message_buffer_append(message, &data[i], 1);
		Message_parse(message);
	}
	TEST_ASSERT_EQUAL_STRING(data, message->parser_state->buffer);
	TEST_ASSERT_EQUAL_INT(strlen(data), message->parser_state->iterations);
	Message_parse_finish(message);

	TEST_ASSERT_EQUAL_INT(2, message->protocol_values->length);
	TEST_ASSERT_EQUAL_STRING("VERIFY", List_get(message->protocol_values, 0));
	TEST_ASSERT_EQUAL_STRING("/foo/bar", List_get(message->protocol_values, 1));

	TEST_ASSERT_EQUAL_INT(2, message->headers->length);
	Pair *header1 = (Pair*)List_get(message->headers, 0);
	TEST_ASSERT_EQUAL_STRING("Header1", header1->key);
	TEST_ASSERT_EQUAL_STRING("value1", header1->value);

	Pair *header2 = (Pair*)List_get(message->headers, 1);
	TEST_ASSERT_EQUAL_STRING("Header2", header2->key);
	TEST_ASSERT_EQUAL_STRING("value2", header2->value);

	TEST_ASSERT_EQUAL_STRING("Hello World", message->body);

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
