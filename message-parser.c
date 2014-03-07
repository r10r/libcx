#include "parser.h"

#define BUFFER_SIZE 16384

int
main(int argc, char **argv)
{
	char buf[BUFFER_SIZE];
	int count = 0;

	while (true)
	{
		int read = fread(&(buf[count]), sizeof(char), BUFFER_SIZE, stdin);
		count += read;

		XFLOG("Read %d\n", read);

		if (read != BUFFER_SIZE)
		{
			if (feof(stdin) )
				break;
			else
			{
				XFLOG("ERROR: %s\n", strerror(ferror(stdin)));
				exit(-1);
			}
		}
		fclose(stdin);
	}

	Message message;
	Message_new(&message);
	Message_parse(&message, buf, count);
	Message_print_stats(&message, stdout);
	Message_free(&message);
}
