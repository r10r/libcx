#include "base/test.h"
#include "handshake.h"

static void
test_Handshake_parse()
{
	FILE* file = fopen("./socket/ws/ws_handshake.txt", "r");

	assert(file);

	StringBuffer* buffer_in = StringBuffer_new(2048);
	StringBuffer_fload(buffer_in, file, 512);
	fclose(file);

	WebsocketsHandshake* handshake = WebsocketsHandshake_new();
	int result = WebsocketsHandshake_parse(handshake, buffer_in);

	Message_fwrite(handshake->message, stdout);

	if (result == -1)
		XFDBG("Handshake parse error: %s", StringBuffer_value(handshake->error_message));
	TEST_ASSERT_EQUAL_INT(1, result);

	StringBuffer* buffer_out = StringBuffer_new(2048);
	WebsocketsHandshake_reply(handshake, buffer_out);
	XFDBG("Handshake Reply:\n%s", StringBuffer_value(buffer_out));

	StringBuffer_free(buffer_in);
	StringBuffer_free(buffer_out);
	WebsocketsHandshake_free(handshake);
}

static void
test_cpu_endian()
{
	TEST_ASSERT_EQUAL_INT(little_endian, CheckCPUEndian());
}

static void
test_StringBuffer_cat_htons()
{
	StringBuffer* buf = StringBuffer_new(1024);
	uint16_t num = 0x1122;

	StringBuffer_cat_htons(buf, num);

	uint16_t* val = (uint16_t*)StringBuffer_value(buf);
	uint16_t decoded = ntohs(*val);
	TEST_ASSERT_TRUE(num == decoded);

	StringBuffer_free(buf);
}

static void
test_StringBuffer_cat_htonl()
{
	StringBuffer* buf = StringBuffer_new(1024);
	uint32_t num = 0x11223344;

	StringBuffer_cat_htonl(buf, num);

	uint32_t* val = (uint32_t*)StringBuffer_value(buf);
	uint32_t decoded = ntohl(*val);
	TEST_ASSERT_TRUE(num == decoded);

	StringBuffer_free(buf);
}

static void
test_StringBuffer_cat_hton64()
{
	StringBuffer* buf = StringBuffer_new(1024);
	uint64_t num = 0x1122334455667788;

	StringBuffer_cat_hton64(buf, num);

	uint64_t* val = (uint64_t*)StringBuffer_value(buf);
	uint64_t decoded = ntoh64(*val);
	TEST_ASSERT_TRUE(num == decoded);

	StringBuffer_free(buf);
}

int
main()
{
	TEST_BEGIN

	RUN(test_Handshake_parse);
	RUN(test_cpu_endian);
	RUN(test_StringBuffer_cat_htons);
	RUN(test_StringBuffer_cat_htonl);
	RUN(test_StringBuffer_cat_hton64);

	TEST_END
}
