#include "libcx-base/debug.h"
#include "message.h"

int
main(int argc, char **argv)
{
	Message *message = Message_new(0);

	Message_buffer_read(message, fileno(stdin), (size_t)atoi(argv[1]));
	Message_parse_finish(message);
	// do something with the message
	return 0;
}
