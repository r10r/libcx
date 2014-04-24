#ifndef _CX_MESSAGE_H
#define _CX_MESSAGE_H

#include <string.h>     /* memcpy, strerror, */
#include <stdio.h>      /* fprintf, feof, fopen, FILE */
#include <stdlib.h>     /* free, exit, malloc */
#include <stdbool.h>    /* true, false */
#include "list/list.h"
#include "string/string_buffer.h"
#include "string/pair.h"
#include "parser.h"

//#define HEADER_LINE_FORMAT "%s: %s\n"
//#define HEADER_LINE_FORMAT_NOV "%s:\n"
//#define HEADER_LINE_FORMAT_LENGTH 3
//#define ENVELOPE_SEPARATOR "\n"

typedef struct message_t Message;

struct message_t
{
	List* protocol_values;  /* list of strings */
	List* headers;          /* list of string pairs */
	StringPointer* body;
	StringBuffer* buffer;
};

Message*
Message_new(void);

void
Message_free(Message* message);

String*
Message_envelope(Message* message);

void
Message_print_stats(Message* message, FILE* file);

ssize_t
Message_read(Message* message, FILE* file, size_t chunk_size);

#endif
