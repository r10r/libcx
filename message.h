#ifndef _MESSAGE_H
#define _MESSAGE_H

#include <string.h>     /* memcpy, strerror, */
#include <stdio.h>      /* fprintf, feof, fopen, FILE */
#include <stdlib.h>     /* free, exit, malloc */
#include <stdbool.h>    /* true, false */
#include "libcx-list/list.h"
#include "libcx-string/string.h"
#include "libcx-string/pair.h"

typedef struct message_t Message;
extern void ragel_parse_message(Message *message);

typedef enum parse_event_t
{
	P_NONE,
	P_PROTOCOL_VALUE,
	P_HEADER_NAME,
	P_HEADER_VALUE,
	P_BODY,
	P_ERROR_MESSAGE_MALFORMED
} ParseEvent;

typedef struct ragel_parser_state_t RagelParserState;
typedef void F_MessageEventHandler (Message *message);

struct ragel_parser_state_t
{
	F_MessageEventHandler *f_event_handler;
	StringBuffer *buffer;  /* message buffer */
	char *buffer_position;
	char *buffer_end;
	char *eof;

	int res;
	int cs;

	size_t buffer_offset;   /* offset from start of buffer */
	size_t marker_start;    /* offset from start of buffer */
	size_t marker_length;   /* marker length */

	/* statistics */
	unsigned int iterations; /* the parser iteration */

	ParseEvent event;

};

struct message_t
{
	List *protocol_values;  /* list of strings */
	List *headers;          /* list of string pairs */
	StringBuffer *body;
	RagelParserState *parser_state;
};

RagelParserState*
RagelParserState_new(unsigned int buffer_length);

void
RagelParserState_free(RagelParserState *state);

Message *
Message_new(unsigned int body_length);

void
Message_free(Message *message);

String*
Message_envelope(Message *message);

void
Message_print_stats(Message *message, FILE *file);

ParseEvent
Message_parse(Message *message);

// set parser state EOF and run last parse
// return 0 if finished successful 1 else
ParseEvent
Message_parse_finish(Message *message);

void
Message_buffer_append(Message *message, const char *buf, size_t count);

#endif
