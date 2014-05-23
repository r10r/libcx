#include "message_parser.h"

int
main(int argc, char** argv)
{
	XASSERT(argc == 2, "Usage: $0 <message>");
	MessageParser* parser = MessageParser_fread(argv[1]);
	Message* message = parser->message;

	size_t nunparsed = RagelParser_unparsed((RagelParser*)parser);
	if (nunparsed > 0)
		XFDBG("ERROR: %zu unparsed tokens", nunparsed);
	else
	{
		/* print the message */
		StringBuffer* buffer = StringBuffer_new(2048);
		Message_print(message, buffer);
		XFDBG("%s", StringBuffer_value(buffer));
		StringBuffer_free(buffer);
	}

	Message_free(message);
	MessageParser_free(parser);
	return 0;
}
