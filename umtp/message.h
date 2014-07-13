#ifndef _CX_MESSAGE_H
#define _CX_MESSAGE_H

#include <string.h>     /* memcpy, strerror, */
#include <stdio.h>      /* fprintf, feof, fopen, FILE */
#include <stdbool.h>    /* true, false */

#include <libcx/list/list.h>
#include <libcx/string/string_buffer.h>
#include <libcx/string/pair.h>

#include "parser.h"

typedef struct cx_message_t Message;

#define MESSAGE_LF "\n"

struct cx_message_t
{
	List* protocol_values;  /* list of strings */
	List* headers;          /* list of string pairs */
	StringPointer* body;
	StringBuffer* buffer;
	StringBuffer* error;

	int keep_buffer;        /* do not free buffer when message is freed */
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

#define Message_add_protocol_value(message, value) \
	List_push(message->protocol_values, S_dup(value))

const char*
Message_get_protocol_value(Message* message, unsigned int index);

int
Message_protocol_value_equals(Message* message, unsigned int index, const char* expected, int ignorecase);

void
Message_set_header(Message* message, const char* key, const char* value);

const char*
Message_get_header_value(Message* message, const char* name, bool ignorecase);

StringPair*
Message_get_header(Message* message, const char* name, bool ignorecase);

int
Message_link_header_value(Message* message, const char* name, const char** const destination);

int
Message_header_value_equals(Message* message, const char* name, const char* value, bool ignorecase);

ssize_t
Message_write(Message* message, int fd);

#define Message_fwrite(message, stream) \
	Message_write(message, fileno(stream))

#endif
