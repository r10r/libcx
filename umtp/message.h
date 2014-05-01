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

typedef struct message_t Message;

#define MESSAGE_LF "\n"

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

void
Message_print(Message* message, StringBuffer* buffer);

void
Message_print_envelope(Message* message, StringBuffer* buffer);

ssize_t
Message_read(Message* message, FILE* file, size_t chunk_size);

#endif
