#ifndef _CX_MESSAGE_H
#define _CX_MESSAGE_H

#include <string.h>     /* memcpy, strerror, */
#include <stdio.h>      /* fprintf, feof, fopen, FILE */
#include <stdlib.h>     /* free, exit, malloc */
#include <stdbool.h>    /* true, false */
#include "libcx-list/list.h"
#include "libcx-string/string.h"
#include "libcx-string/pair.h"
#include "parser.h"

//#define HEADER_LINE_FORMAT "%s: %s\n"
//#define HEADER_LINE_FORMAT_NOV "%s:\n"
//#define HEADER_LINE_FORMAT_LENGTH 3
//#define ENVELOPE_SEPARATOR "\n"

typedef struct message_t Message;

typedef enum parse_event_t
{
	P_NONE,
	P_PROTOCOL_VALUE,
	P_HEADER_NAME,
	P_HEADER_VALUE,
	P_BODY,
	P_ERROR_MESSAGE_MALFORMED
} ParseEvent;

struct message_t
{
	List *protocol_values;  /* list of strings */
	List *headers;          /* list of string pairs */
	StringPointer *body;
	StringBuffer *buffer;
};

Message *
Message_new(void);

void
Message_free(Message *message);

String*
Message_envelope(Message *message);

void
Message_print_stats(Message *message, FILE *file);

RagelParser *
MessageParser_new(size_t buffer_size);

ssize_t
Message_read(Message *message, FILE *file, size_t chunk_size);

Message *
Message_fread(const char *file_path);

#endif
