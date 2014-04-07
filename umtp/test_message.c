#include "base/test.h"
#include "message_parser.h"

static void
test_Message_new()
{
	Message* message = Message_new();

	TEST_ASSERT_EQUAL_INT(0, message->protocol_values->length);
	TEST_ASSERT_EQUAL_INT(0, message->headers->length);
	TEST_ASSERT_NULL(message->body);
	TEST_ASSERT_NULL(message->buffer);

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
test_assert_message(Message* message)
{
	TEST_ASSERT_EQUAL_S(data, message->buffer->string);

	TEST_ASSERT_EQUAL_INT(2, message->protocol_values->length);
	TEST_ASSERT_EQUAL_S("VERIFY", List_S(message->protocol_values, 0));
	TEST_ASSERT_EQUAL_S("/foo/bar", List_S(message->protocol_values, 1));

	TEST_ASSERT_EQUAL_INT(2, message->headers->length);
	StringPair* header1 = (StringPair*)List_get(message->headers, 0);
	TEST_ASSERT_EQUAL_S("Header1",  header1->key);
	TEST_ASSERT_EQUAL_S("value1",  header1->value);

	StringPair* header2 = (StringPair*)List_get(message->headers, 1);
	TEST_ASSERT_EQUAL_S("Header2",  header2->key);
	TEST_ASSERT_EQUAL_S("value2",  header2->value);
	TEST_ASSERT_EQUAL_S("Hello World", message->body);
}

static void
test_Message_parse_single_pass()
{
	MessageParser* parser = MessageParser_new(strlen(data));
	RagelParser* ragel_parser = (RagelParser*)parser;

	StringBuffer_ncat(ragel_parser->buffer, data, strlen(data));
	RagelParser_finish(ragel_parser);
	test_assert_message(parser->message);

	Message_free(parser->message);
	MessageParser_free(parser);
}

static void
test_Message_parse_multi_pass()
{
	MessageParser* parser = MessageParser_new(1);
	RagelParser* ragel_parser = (RagelParser*)parser;

	unsigned int i;

	for (i = 0; i < strlen(data); i++)
	{
		StringBuffer_ncat(ragel_parser->buffer, &data[i], 1);

		if (i == (strlen(data) - 1))
			RagelParser_finish(ragel_parser)
			else
				RagelParser_parse(ragel_parser);
	}

	Message* message = parser->message;
	test_assert_message(message);

	Message_free(message);
	MessageParser_free(parser);
}

static void
test_Message_read_file()
{
	Message* message = MessageParser_fread("umtp/testmessages/hello_world.txt");

	test_assert_message(message);
	Message_free(message);
}

int
main()
{
	TEST_BEGIN

	RUN(test_Message_new);
	RUN(test_Message_parse_single_pass);
	RUN(test_Message_parse_multi_pass);
	RUN(test_Message_read_file);

	TEST_END
}
