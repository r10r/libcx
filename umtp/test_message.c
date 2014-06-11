#include "../base/test.h"
#include "message_parser.h"

#define List_S(list, index) \
	((String*)List_get(list, index))->value

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

static const char data[] =
	"VERIFY /foo/bar\n"
	"Header1: value1\n"
	"Header2: value2\n"
	"\n"
	"Hello World";

static void
test_assert_message(Message* message)
{
	TEST_ASSERT_EQUAL_STRING(data, StringBuffer_value(message->buffer));

	TEST_ASSERT_EQUAL_INT(2, message->protocol_values->length);
	TEST_ASSERT_EQUAL_STRING("VERIFY", List_S(message->protocol_values, 0));
	TEST_ASSERT_EQUAL_STRING("/foo/bar", List_S(message->protocol_values, 1));

	TEST_ASSERT_EQUAL_INT(2, message->headers->length);
	StringPair* header1 = (StringPair*)List_get(message->headers, 0);
	TEST_ASSERT_EQUAL_STRING("Header1",  header1->key->value);
	TEST_ASSERT_EQUAL_STRING("value1",  header1->value->value);

	StringPair* header2 = (StringPair*)List_get(message->headers, 1);
	TEST_ASSERT_EQUAL_STRING("Header2",  header2->key->value);
	TEST_ASSERT_EQUAL_STRING("value2",  header2->value->value);

	TEST_ASSERT_EQUAL_STRING("Hello World", message->body->value);
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
	MessageParser* parser = MessageParser_fread("umtp/testmessages/hello_world.txt");

	TEST_ASSERT_EQUAL_INT(0, RagelParser_unparsed((RagelParser*)parser));

	Message* message = parser->message;
	test_assert_message(message);
	Message_free(message);
	MessageParser_free(parser);
}

static void
test_Message_protocol_value_equals()
{
	Message* message = Message_new();

	Message_add_protocol_value(message, "GET");
	Message_add_protocol_value(message, "/foo");
	Message_add_protocol_value(message, "HTTP/1.1");

	TEST_ASSERT_EQUAL_INT(1, Message_protocol_value_equals(message, 0, "GET", 0));
	TEST_ASSERT_EQUAL_INT(1, Message_protocol_value_equals(message, 1, "/foo", 0));
	TEST_ASSERT_EQUAL_INT(1, Message_protocol_value_equals(message, 2, "HTTP/1.1", 0));

	TEST_ASSERT_EQUAL_INT(1, Message_protocol_value_equals(message, 2, "http/1.1", 1));

	Message_free(message);
}

static void
test_Message_header_value_equals()
{
	Message* message = Message_new();

	Message_set_header(message, "foo", "bar");

	TEST_ASSERT_NOT_NULL(Message_get_header(message, "foo", false));
	TEST_ASSERT_EQUAL_INT(1, message->headers->length);

	/* case sensitive */
	TEST_ASSERT_EQUAL_INT(1, Message_header_value_equals(message, "foo", "bar", 0));
	TEST_ASSERT_EQUAL_INT(0, Message_header_value_equals(message, "foo", "Bar", 0));

	/* case insensitive */
	TEST_ASSERT_EQUAL_INT(1, Message_header_value_equals(message, "foo", "bar", 1));
	TEST_ASSERT_EQUAL_INT(1, Message_header_value_equals(message, "foo", "Bar", 1));

	/* overwrite header */
	Message_set_header(message, "foo", "blubber");
	TEST_ASSERT_EQUAL_INT(1, message->headers->length);

	TEST_ASSERT_EQUAL_INT(1, Message_header_value_equals(message, "foo", "blubber", 0));

	const char* link;
	Message_set_header(message, "xxx", "yyy");
	Message_link_header_value(message, "xxx", &link);
	TEST_ASSERT_EQUAL_STRING(link, "yyy");

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
	RUN(test_Message_protocol_value_equals);
	RUN(test_Message_header_value_equals);

	TEST_END
}
