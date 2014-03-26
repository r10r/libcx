#include "libcx-base/debug.h"
#include "message.h"

int
main(int argc, char **argv)
{
	XASSERT(argc == 2, "Usage: $0 <message>");
	Message *message = Message_fread(argv[1]);
	// do something useful with the message
	Message_free(message);
	return 0;
}
