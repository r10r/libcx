#include "parser.h"

/*
 * @optimize memory
 * Buffer might be shifted up to last marker position or buffer offset
 * (the one which is larger). both marker offset and position must be
 * recalculated. Don't know whether this makes sense.
 * Test whether shifting the buffer or double buffering (start a new
 * buffer for each marker) is better.
 */

void
RagelParser_init(RagelParser* parser)
{
	/* non-ragel state */
	parser->buffer = NULL;
	parser->buffer_offset = 0;
	parser->marker_start = 0;
	parser->marker_length = 0;
	parser->f_event = NULL;
	parser->iterations = 0;
	parser->finished = 0;
	parser->initialized = 0;

	/* ragel machine state */
	parser->res = 0;
	parser->cs = 0;

	/* ragel buffer state */
	parser->buffer_position = NULL;
	parser->buffer_end = NULL;
	parser->eof = NULL;
	parser->f_parse = NULL;

	parser->userdata = NULL;
}

RagelParser*
RagelParser_new()
{
	RagelParser* parser = cx_alloc(sizeof(RagelParser));

	RagelParser_init(parser);
	return parser;
}

void
RagelParser_free(RagelParser* parser)
{
	cx_free(parser);
}

int
RagelParser_firstrun(RagelParser* parser)
{
	if (parser->initialized)
		return 0;
	else
	{
		parser->initialized = 1;
		parser->iterations = 0;
		return 1;
	}
}

void
RagelParser_parse_file(RagelParser* parser, const char* file_path, size_t chunk_size)
{
	FILE* file = fopen(file_path, "r");

	XFASSERT(file, "file %s should exist", file_path);
	ssize_t nread = StringBuffer_fload(parser->buffer, file, chunk_size);
	fclose(file);

	if (nread <= 0)
		XFLOG("error while reading from file : %s : %s", file_path, strerror(errno));

	RagelParser_finish(parser);
}

ssize_t
RagelParser_fdparse(RagelParser* parser, int fd, size_t chunk_size)
{
	ssize_t total_read = 0;
	StringBuffer* buffer = StringBuffer_new(chunk_size);

	parser->buffer = buffer;

	while (1)
	{
		ssize_t nread = StringBuffer_fdncat(buffer, fd, chunk_size);
		total_read += nread;

		if (nread == 0)
		{
			RagelParser_finish(parser);
			break;
		}
		if (nread < 0)
		{
			total_read = nread;
			break;
		}
		RagelParser_parse(parser);
	}
	StringBuffer_free(buffer);
	return total_read;
}

void
RagelParser_parse(RagelParser* parser)
{
	F_ParseHandler* current_handler = parser->f_parse;

	parser->f_parse(parser);
	F_ParseHandler* next_handler = parser->f_parse;

	// check if parser has changed and if we have any remaining tokens
	if (next_handler != current_handler)
	{
		XDBG("Parser has changed");
		parser->initialized = 0;        // triggers RagelParser_firstrun
		size_t nunparsed =  RagelParser_unparsed(parser);
		if (nunparsed > 0)
		{
			XFDBG("%zu unparsed tokens. calling body parser", nunparsed);
			RagelParser_parse(parser);
		}
	}
}

void
RagelParser_shift_buffer(RagelParser* parser)
{
	int ret = StringBuffer_shift(parser->buffer, parser->buffer_offset);

	if (ret == 1)
	{
		XFDBG("Shifted buffer by %zu tokens", parser->buffer_offset);
		parser->buffer_offset = 0;
		parser->marker_length = 0;
		parser->marker_start = 0;
		RagelParser_update(parser);
	}
	else
		XDBG("Failed to shift buffer");
}

size_t
RagelParser_unparsed(RagelParser* parser)
{
	long nunparsed = parser->buffer_end - parser->buffer_position;

	assert(nunparsed >= 0); /* memory corruption if negative */
	return (size_t)nunparsed;
}
