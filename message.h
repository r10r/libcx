#ifndef _MESSAGE_H
#define _MESSAGE_H

#include <string.h>     /* memcpy, strerror, */
#include <stdio.h>      /* fprintf, feof, fopen, FILE */
#include <stdlib.h>     /* free, exit, malloc */
#include <stdbool.h>    /* true, false */
#include "libcx-list/list.h"
#include "libcx-string/string.h"

typedef struct message_t Message;

typedef enum parse_event_t
{
	P_NEW,
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
	char *buffer_position;
	char *buffer_end;
	unsigned int buffer_offset;
	char *eof;
	int res;
	int cs;
	char *marker;
	unsigned int marker_offset;
	ParseEvent event;
	F_MessageEventHandler *f_event_handler;
};

struct message_t
{
	List *protocol_values;  /* list of strings */
	List *headers;          /* list of string pairs */
	String buffer;          /* message buffer */
	String body;
	RagelParserState *parser_state;
};

static inline size_t Message_size(Message *message)
{
	return 0;
}

// mark begin of body to track envelope  and header size
static inline size_t Message_body_size(Message *message)
{
	return 0;
}

Message *
Message_new(size_t body_size);

void
Message_free(Message *message);

String
Message_envelope(Message *message);

void
Message_print_stats(Message *message, FILE *file);

ParseEvent
Message_parse(Message *message);

// set parser state EOF and run last parse
// return 0 if finished successful 1 else
ParseEvent
Message_parse_finish(Message *message);

#endif
