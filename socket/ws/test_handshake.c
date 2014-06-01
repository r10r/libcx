#include "base/test.h"
#include "handshake.h"

typedef enum _endian { little_endian, big_endian } EndianType;

static EndianType
CheckCPUEndian()
{
	unsigned short x;
	unsigned char c;
	EndianType CPUEndian;

	x = 0x0001;
	;
	c = *(unsigned char*)(&x);
	if ( c == 0x01 )
		CPUEndian = little_endian;
	else
		CPUEndian = big_endian;

	return CPUEndian;
}

static void
test_Handshake_parse()
{
	FILE* file = fopen("/Users/ruben/Code/libcx/socket/ws/ws_handshake.txt", "r");

	assert(file);

	StringBuffer* buffer_in = StringBuffer_new(2048);
	StringBuffer_fload(buffer_in, file, 512);
	fclose(file);

	WebsocketsHandshake* handshake = WebsocketsHandshake_new();
	int result = WebsocketsHandshake_parse(handshake, buffer_in);

	Message_fwrite(handshake->message, stdout);

	if (result == -1)
		printf("Handshake parse error: %s\n", StringBuffer_value(handshake->error_message));
	TEST_ASSERT_EQUAL_INT(1, result);

	StringBuffer* buffer_out = StringBuffer_new(2048);
	WebsocketsHandshake_reply(handshake, buffer_out);
	printf("%s\n", StringBuffer_value(buffer_out));

	StringBuffer_free(buffer_in);
	StringBuffer_free(buffer_out);
	WebsocketsHandshake_free(handshake);
}

//#define WS_HDR(bits, opcode, payload_len) \
// //	bits | (opcode << WS_HDR_OPCODE) | (payload_len << WS_HDR_PAYLOAD_LENGTH);

static void
test_cpu_endian()
{
	switch (CheckCPUEndian())
	{
	case little_endian:
		printf("Little Endian\n");
		break;
	case big_endian:
		printf("Big Endign\n");
		break;
	}
}

int
main()
{
	TEST_BEGIN

	RUN(test_Handshake_parse);
	RUN(test_cpu_endian);

	TEST_END
}
