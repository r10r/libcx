#include "libcx-base/test.h"
#include "message.h"
#include "parser.h"

NOSETUP

void test_new()
{
	Message msg;

	Message_new(&msg);

	TEST_ASSERT_NULL(msg.body);
	TEST_ASSERT_NULL(msg.data);
	TEST_ASSERT_NULL(msg.method);
	TEST_ASSERT_NULL(msg.sender);
	TEST_ASSERT_NULL(msg.topic);
	TEST_ASSERT_EQUAL_INT(0, msg.body_length);
	TEST_ASSERT_EQUAL_INT(0, msg.data_size);

	TEST_ASSERT_NOT_NULL(&msg.envelope);
	TEST_ASSERT_NULL(msg.envelope.first);
	TEST_ASSERT_NULL(msg.envelope.last);
	TEST_ASSERT_EQUAL_INT(0, msg.envelope.header_count);
}

void test_set_header()
{
	Message msg;

	Message_new(&msg);
	Header *header = Message_set_header(&msg, METHOD, HEADER_NAME[METHOD], METHOD_NAME[PUBLISH]);

	TEST_ASSERT_NOT_NULL(header);
	TEST_ASSERT_EQUAL_INT(1, msg.envelope.header_count);
	TEST_ASSERT_EQUAL_PTR(header, msg.envelope.first);
	TEST_ASSERT_EQUAL(header, msg.envelope.last);
	TEST_ASSERT_NULL(header->next);

	TEST_ASSERT_EQUAL_STRING("Method", header->name);
	TEST_ASSERT_EQUAL_STRING("PUBLISH", header->value);
	TEST_ASSERT_EQUAL_STRING(METHOD, header->type);

	Message_free(&msg);
}

void test_set_topic()
{
	Message msg;

	Message_new(&msg);
	Message_set_topic(&msg, "/foo/bar");

	TEST_ASSERT_EQUAL_INT(1, msg.envelope.header_count);
	TEST_ASSERT_NOT_NULL(msg.topic);
	TEST_ASSERT_EQUAL(TOPIC, msg.topic->type);
	TEST_ASSERT_EQUAL_STRING("Topic", msg.topic->name);
	TEST_ASSERT_EQUAL_STRING("/foo/bar", msg.topic->value);

	Message_free(&msg);
}

void test_set_sender()
{
	Message msg;

	Message_new(&msg);
	Message_set_sender(&msg, "/foo/bar");

	TEST_ASSERT_EQUAL_INT(1, msg.envelope.header_count);
	TEST_ASSERT_NOT_NULL(msg.sender);
	TEST_ASSERT_EQUAL(SENDER, msg.sender->type);
	TEST_ASSERT_EQUAL_STRING("Sender", msg.sender->name);
	TEST_ASSERT_EQUAL_STRING("/foo/bar", msg.sender->value);

	Message_free(&msg);
}

void test_set_method()
{
	Message msg;

	Message_new(&msg);
	Message_set_method(&msg, PUBLISH);

	TEST_ASSERT_EQUAL_INT(1, msg.envelope.header_count);
	TEST_ASSERT_NOT_NULL(msg.method);
	TEST_ASSERT_EQUAL(METHOD, msg.method->type);
	TEST_ASSERT_EQUAL_STRING("Method", msg.method->name);
	TEST_ASSERT_EQUAL_STRING("PUBLISH", msg.method->value);

	Message_free(&msg);
}

void test_SetMultipleHeaders()
{
	Message msg;
	char *data =
		"\nTopic: /foo/bar\n"
		"Sender: xyz\n"
		"Method: PUBLISH\n"
		"Foo: Bar\n"
		"\n"
		"Hello World";

	Message_new(&msg);

	int data_parsed = Message_parse(&msg, data, strlen(data));
	TEST_ASSERT_EQUAL_INT(strlen(data), data_parsed);
	TEST_ASSERT_EQUAL_INT(4, msg.envelope.header_count);
	TEST_ASSERT_EQUAL_STRING(msg.topic->value, "/foo/bar");
	TEST_ASSERT_EQUAL_STRING(msg.sender->value, "xyz");
	TEST_ASSERT_EQUAL_STRING(msg.method->value, "PUBLISH");
	TEST_ASSERT_EQUAL_STRING(msg.envelope.last->value, "Bar");
	TEST_ASSERT_EQUAL_STRING(msg.body, "Hello World");
}

void test_write_to_buffer()
{
	Message msg;
	char *data =
		"PUBLISH /foo/bar\n"
		"Topic: /foo/bar\n"
		"Sender: xyz\n"
		"Method: PUBLISH\n"
		"Foo: Bar\n"
		"\n"
		"Hello World";

	Message_new(&msg);
	Message_parse(&msg, data, strlen(data));
	Message_print_stats(&msg, stderr);
	TEST_ASSERT_EQUAL_INT(strlen(data), Message_length(&msg));

	char buf[Message_length(&msg)];
	int write_count = Message_write_to_buffer(&msg, buf);
	TEST_ASSERT_EQUAL_INT( Message_length(&msg), write_count);
	TEST_ASSERT_EQUAL_STRING(data, buf);

	Message_free(&msg);
}

void test_write_to_file()
{
	Message msg;
	char *data =
		"PUBLISH\n"
		"Topic: /foo/bar\n"
		"Sender: xyz\n"
		"Method: PUBLISH\n"
		"Foo: Bar\n"
		"\n"
		"Hello World";

	Message_new(&msg);
	Message_parse(&msg, data, strlen(data));
	TEST_ASSERT_EQUAL_INT(strlen("Hello World"), msg.body_length);
	TEST_ASSERT_EQUAL_INT(strlen(data), Message_write_to_file(&msg, stdout));
	Message_free(&msg);
}

int main()
{
	TEST_BEGIN

	RUN(test_new);
	RUN(test_set_header);
	RUN(test_set_topic);
	RUN(test_set_sender);
	RUN(test_set_method);
	RUN(test_SetMultipleHeaders);
	RUN(test_write_to_buffer);
	RUN(test_write_to_file);

	TEST_END
}
