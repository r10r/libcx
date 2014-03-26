#include "parser.h"
/*
 * @optimize memory
 * Buffer might be shifted up to last marker position or buffer offset
 * (the one which is larger). both marker offset and position must be
 * recalculated. Don't know whether this makes sense.
 * Test whether shifting the buffer or double buffering (start a new
 * buffer for each marker) is better.
 */

/*
 * string referenced by buffer might have been reallocated
 * restore the buffer position
 */

RagelParser*
RagelParser_new()
{
	RagelParser *parser = malloc(sizeof(RagelParser));

	/* non-ragel state */
	parser->buffer = NULL;
	parser->buffer_offset = 0;
	parser->marker_start = 0;
	parser->marker_length = 0;
	parser->f_event_handler = NULL;
	parser->iterations = 0;
	parser->finished = 0;
	parser->userdata = NULL;

	/* ragel state */
	parser->res = 0;
	parser->cs = 0;
	parser->buffer_position = NULL;
	parser->buffer_end = NULL;
	parser->eof = NULL;

	return parser;
}

void
RagelParser_free(RagelParser *parser)
{
	free(parser);
}

void *
RagelParser_parse_file(RagelParser *parser, const char *file_path, size_t chunk_size)
{
	void *data = parser->userdata;
	FILE *file = fopen(file_path, "r");

	XFASSERT(file, "file %s should exist\n", file_path);
	ssize_t nread = StringBuffer_fload(parser->buffer, file, chunk_size);
	fclose(file);

	if (nread <= 0)
		XFLOG("error while reading from file : %s : %s", file_path, strerror(errno));

	RagelParser_finish(parser);
	return data;
}
