#include "message.h"

int
main(int argc, char **argv)
{
	Message *message = Message_new(1024);
	int status = 0;

	while (true)
	{
		message->buffer = String_append_stream(message->buffer, stdin, 1024);

		if (feof(stdin))
		{
			status = Message_parse_finish(message);
			break;
		}
		else
			Message_parse(message);
	}

	return status;
}
