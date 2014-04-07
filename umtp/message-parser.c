#include "base/debug.h"
#include "message_parser.h"

int
main(int argc, char** argv)
{
	XASSERT(argc == 2, "Usage: $0 <message>");
	Message* message = MessageParser_fread(argv[1]);
	// do something useful with the message
	Message_free(message);
	return 0;
}
