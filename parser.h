#ifndef _CX_PARSER
#define _CX_PARSER

#include "libcx-string/string.h"

#define Marker_get(parser) \
	& S_get(parser->buffer->string, parser->marker_start)

#define Marker_toS(parser) \
	String_init(Marker_get(parser), parser->marker_length)

#define PrintToken(parser) \
	XFLOG("Token[%zu] %c -> %p\n", \
	      parser->buffer_offset, *(parser->buffer_position), parser->buffer_position);

#define SetMarker(parser) \
	XFLOG("Mark[%zu] %c -> %p\n", \
	      parser->buffer_offset, *(parser->buffer_position), parser->buffer_position); \
	parser->marker_start = parser->buffer_offset; \
	parser->marker_length = 0; \

#define CountToken(parser) \
	parser->buffer_offset++; \
	parser->marker_length++;

#define EmitEvent(parser, e) \
	parser->f_event(parser, e);

#define RagelParser_update(parser) \
	parser->buffer_position = &S_get(parser->buffer->string, parser->buffer_offset); \
	parser->buffer_end = &S_last(parser->buffer->string); \
	if (parser->finished == 1) parser->eof = parser->buffer_end; \

#define RagelParser_finish(parser) \
	{ parser->finished = 1; RagelParser_parse(parser); }

#define RagelParser_eof(parser) \
	(parser->eof == parser->buffer_end)

/* number of unparsed tokens */
#define RagelParser_unparsed(parser) \
	(parser->buffer->string->length - parser->buffer_offset)

typedef struct ragel_parser_t RagelParser;
typedef void F_EventHandler (RagelParser *parser, int event);
typedef void F_ParseHandler (RagelParser *parser);

struct ragel_parser_t
{
	/* non-ragel state */
	F_EventHandler *f_event;        /* handles parsing events */
	F_ParseHandler *f_parse;        /* the parse function defined in the *.rl file */
	StringBuffer *buffer;           /* input buffer */
	size_t buffer_offset;           /* offset from start of buffer */
	size_t marker_start;            /* offset from start of buffer */
	size_t marker_length;           /* marker length */
	int finished;                   /* indicate eof */
	unsigned int iterations;        /* the parser iteration */

	/* ragel state */
	char *buffer_position;
	char *buffer_end;
	char *eof;
	int res;
	int cs;
};

void
RagelParser_parse(RagelParser *parser);

void
RagelParser_init(RagelParser *parser);

RagelParser*
RagelParser_new(void);

void
RagelParser_free(RagelParser *parser);

void
RagelParser_parse_file(RagelParser *parser, const char *file_path, size_t chunk_size);

#endif
