#include "libcx-base/test.h"
#include "message.h"

NOSETUP

void test_Message_new()
{
	Message *message = Message_new(1024);

	TEST_ASSERT_EQUAL_INT(0, String_length(message->buffer));
	TEST_ASSERT_EQUAL_INT(1024, String_available(message->buffer));
	TEST_ASSERT_EQUAL_INT(0, message->protocol_values->length);
	TEST_ASSERT_EQUAL_INT(0, message->headers->length);
	Message_free(message);
}

void test_Message_parse_single_pass()
{
	Message *message =  Message_new(1024);

	const char *data =
		"VERIFY /foo/bar\n"
		"Header1: value1\n"
		"Header2: value2\n"
		"\n"
		"Hello World";

	message->buffer = String_append_constant(message->buffer, data);
	ParseEvent event = Message_parse_finish(message);

	TEST_ASSERT_EQUAL_STRING(data, message->buffer);

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

void test_Message_parse_multi_pass()
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
		message->buffer = String_append_array(message->buffer, &data[i], 1);
		Message_parse(message);
	}
	TEST_ASSERT_EQUAL_STRING(data, message->buffer);

	ParseEvent event = Message_parse_finish(message);

	TEST_ASSERT_EQUAL_INT(2, message->protocol_values->length);
//	TEST_ASSERT_EQUAL_STRING("VERIFY", List_get(message->protocol_values, 0));
//	TEST_ASSERT_EQUAL_STRING("/foo/bar", List_get(message->protocol_values, 1));
//
//	TEST_ASSERT_EQUAL_INT(2, message->headers->length);
//	Pair *header1 = (Pair*)List_get(message->headers, 0);
//	TEST_ASSERT_EQUAL_STRING("Header1", header1->key);
//	TEST_ASSERT_EQUAL_STRING("value1", header1->value);
//
//	Pair *header2 = (Pair*)List_get(message->headers, 1);
//	TEST_ASSERT_EQUAL_STRING("Header2", header2->key);
//	TEST_ASSERT_EQUAL_STRING("value2", header2->value);

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
