#ifndef _CX_PARSER_H
#define _CX_PARSER_H

#include <libcx/string/string_buffer.h>

#ifdef _CX_TRACE
#define ParserDebug(parser, msg) \
	XFDBG("\n" \
	      "-- parser debug [ %s ] --\n" \
	      "state	[ cs:%d iterations:%d ]\n" \
	      "token	[ pos:%p end:%p eof:%p ]\n" \
	      "buffer	[ offset:%zu length:%zu used:%zu unused:%zu ]\n" \
	      "marker	[ start:%zu length:%zu ]\n" \
	      "-------------------------------", \
	      msg, \
	      (parser)->cs, (parser)->iterations, \
	      (parser)->buffer_position, (parser)->buffer_end, (parser)->eof, \
	      (parser)->buffer_offset, StringBuffer_length((parser)->buffer), \
	      StringBuffer_used((parser)->buffer), StringBuffer_unused((parser)->buffer), \
	      (parser)->marker_start, (parser)->marker_length);

// TODO make unprintable tokens printable (for easier debugging)
#define PrintToken(parser) \
	XFDBG("Token[%zu] %p (%d)'%c'\n", \
	      (parser)->buffer_offset, (parser)->buffer_position, \
	      *((parser)->buffer_position), *((parser)->buffer_position));

#define PrintLastToken(parser) \
	XFDBG("Last Token[%zu] %p (%d)'%c'\n", \
	      StringBuffer_used((parser)->buffer), S_last((parser)->buffer->string), \
	      *S_last((parser)->buffer->string), *S_last((parser)->buffer->string));

#else

#define ParserDebug(parser, msg) UNUSED(msg)
#define PrintToken(parser) UNUSED(parser)
#define PrintLastToken(parser) UNUSED(parser)

#endif

#define Marker_get(parser) \
	S_get((parser)->buffer->string, (parser)->marker_start)

#define Marker_toS(parser) \
	String_init(Marker_get(parser), (parser)->marker_length)

#define SetMarker(parser) \
	XFDBG("Mark[%zu] %c -> %p\n", \
	      (parser)->buffer_offset, *((parser)->buffer_position), (parser)->buffer_position); \
	(parser)->marker_start = (parser)->buffer_offset; \
	(parser)->marker_length = 0; \

#define CountToken(parser) \
	(parser)->buffer_offset++; \
	(parser)->marker_length++;

#define EmitEvent(parser, e) \
	(parser)->f_event(parser, e);

#define RagelParser_update(parser) \
	(parser)->buffer_position = S_get((parser)->buffer->string, (parser)->buffer_offset); \
	(parser)->buffer_end = S_last((parser)->buffer->string) + 1; \
	if ((parser)->finished == 1) (parser)->eof = (parser)->buffer_end;

#define RagelParser_finish(parser) \
	{ (parser)->finished = 1; RagelParser_parse(parser); }

#define RagelParser_eof(parser) \
	((parser)->eof == (parser)->buffer_end)

typedef struct cx_ragel_parser_t RagelParser;
typedef void F_EventHandler (RagelParser* parser, int event);
typedef void F_ParseHandler (RagelParser* parser);

struct cx_ragel_parser_t
{
	/* non-ragel state */
	F_EventHandler* f_event;        /* handles parsing events */
	F_ParseHandler* f_parse;        /* the parse function defined in the *.rl file */
	StringBuffer* buffer;           /* input buffer */
	size_t buffer_offset;           /* offset from start of buffer */
	size_t marker_start;            /* offset from start of buffer */
	size_t marker_length;           /* marker length */

	/* ragel state */
	char* buffer_position;
	char* buffer_end;
	char* eof;
	int res;        /* FIXME unused ?) */
	int cs;

	int finished;                   /* indicate eof */
	unsigned int iterations;        /* the parser iteration */
	int initialized;

	void* userdata;
};

void
RagelParser_parse(RagelParser* parser);

void
RagelParser_init(RagelParser* parser);

RagelParser*
RagelParser_new(void);

int
RagelParser_firstrun(RagelParser* parser);

void
RagelParser_free(RagelParser* parser);

void
RagelParser_parse_file(RagelParser* parser, const char* file_path, size_t chunk_size);

void
RagelParser_shift_buffer(RagelParser* parser);

/*
 * Read chunk_size length data from fd and calls parser after each read
 * @return number of bytes read or < 0 on error
 */
ssize_t
RagelParser_fdparse(RagelParser* parser, int fd, ssize_t chunk_size);

/* number of unparsed tokens */
size_t
RagelParser_unparsed(RagelParser* parser);

#endif
