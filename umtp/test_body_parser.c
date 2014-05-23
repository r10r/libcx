#include "base/test.h"
#include "message_parser.h"

extern void
body_fsm_parse(RagelParser* parser);

#define List_S(list, index) \
	((String*)List_get(list, index))->value

static const char data[] =
	"VERIFY /foo/bar\n"
	"Header1: value1\n"
	"\n"
	"XYZ\n"
	"XYZ\n"
	"XYZ\n"
	"XYZ\n"
	"XYZ\n"
	"\n";

static void
test_assert_message(Message* message)
{
	TEST_ASSERT_EQUAL_STRING(data, StringBuffer_value(message->buffer));

	TEST_ASSERT_EQUAL_INT(2, message->protocol_values->length);
	TEST_ASSERT_EQUAL_STRING("VERIFY", List_S(message->protocol_values, 0));
	TEST_ASSERT_EQUAL_STRING("/foo/bar", List_S(message->protocol_values, 1));

	TEST_ASSERT_EQUAL_INT(1, message->headers->length);
	StringPair* header1 = (StringPair*)List_get(message->headers, 0);
	TEST_ASSERT_EQUAL_STRING("Header1",  header1->key->value);
	TEST_ASSERT_EQUAL_STRING("value1",  header1->value->value);
}

static StringBuffer* buf;

static void
event_handler(RagelParser* parser, int event)
{
	UNUSED(event);
	XFDBG("Received Event %d", event);
	StringBuffer_ncat(buf, Marker_get(parser), parser->marker_length);
}

static void
test_Message_parse_multi_pass()
{
	buf =  StringBuffer_new(1024);
	MessageParser* parser = MessageParser_new(1);

	parser->f_body_event = event_handler;
	parser->f_body_parse = body_fsm_parse;
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
	RagelParser_finish(ragel_parser)  // FIXME duplicate call required to run body parser
	test_assert_message(message);

	TEST_ASSERT_EQUAL_STRING("XYZXYZXYZXYZXYZ", buf->string->value);

	StringBuffer_free(buf);
	Message_free(message);
	MessageParser_free(parser);
}

int
main()
{
	TEST_BEGIN

	RUN(test_Message_parse_multi_pass);

	TEST_END
}
