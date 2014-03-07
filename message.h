#ifndef _MESSAGE_H
#define _MESSAGE_H

#include <string.h>     /* memcpy, strerror, */
#include <stdio.h>      /* fprintf, feof, fopen, FILE */
#include <stdlib.h>     /* free, exit, malloc */
#include <stdbool.h>    /* true, false */

typedef enum
{
	/*!< publish a message to a topic */
	PUBLISH,
	/*!< subscribe to a topic */
	SUBSCRIBE,
	/*!< unsubscribe from a topic */
	UNSUBSCRIBE,
	/*!< undefined message topic */
	UNDEFINED
} MethodType;

MethodType
MethodType_of(const char *value);

extern const char *METHOD_NAME[];
extern const char *HEADER_NAME[];

typedef enum
{
	METHOD,
	TOPIC,
	SENDER,
	OTHER
} HeaderType;

HeaderType
HeaderType_of(const char *value);

typedef struct protocol_value_t ProtocolValue;

// FIXME use List implementation here
struct protocol_value_t
{
	char *data;
	int length;
	ProtocolValue *next;
};

// FIXME use List implementation here
typedef struct protocol_line_t
{
	int value_count;
	ProtocolValue *first_value;
	ProtocolValue *last_value;
} ProtocolLine;

// FIXME use list implementation here
/* TODO use ProtocolValue for header values and keys ? */
typedef struct header_t
{
	const char *name;
	const char *value;
	HeaderType type;
	struct header_t *next;
	int name_length;
	int value_length;
} Header;

// FIXME use list implementation here
typedef struct header_list_t
{
	Header *first;
	Header *last;
	int header_count;
} Envelope;

typedef struct parsed_message_t
{
	MethodType type;
	char *data;
	long data_size;
	const char *body;
	ProtocolLine protocol_line;
	long body_length;
	Header *method;         /* pointer to method header */
	Header *topic;          /* pointer to topic header */
	Header *sender;         /* pointer to sender header */
	Envelope envelope;      /* topic and sender are included in the envelope */
} Message;

void
Message_new(Message *message);

void
Message_set_body(Message *message, const char *body, long length);

long
Message_length(Message *msg);

void
Header_new(Header *header, const char *name, const char *value);

void
Header_set_value(Header *header, const char *value);

void
Header_set_name(Header *header, const char *name);

long
Envelope_length(Envelope *envelope);

long
ProtocolLine_length(ProtocolLine *line);

void
ProtocolLine_add_value(ProtocolLine *line, char* data, int length);

/* simply appends the given header */
Header *
Envelope_add_header(Envelope *envelope, const char *name, const char *value);

Header *
Envelope_remove_header(Envelope *envelope, HeaderType type, char *name);

long
Envelope_write_to_file(Envelope *envelope, FILE *file);

long
Envelope_write_to_buffer(Envelope *envelope, char *buf);

long
Message_write_to_buffer(Message *message, char *buf);

long
Message_write_to_file(Message *message, FILE *file);

void
Message_free(Message *message);

/* TODO check for duplicate headers !!! */
Header *
Message_set_header(Message *message, HeaderType type, const char *name, const char *value);

void
Message_detect_header_type(Message *message, Header *header);

void
Message_set_topic(Message *message, const char *topic);

void
Message_set_sender(Message *message, const char *sender);

void
Message_set_method(Message *message, MethodType method);

void
ProtocolValue_new(ProtocolValue *value, char *data, int length);

void
Message_print_stats(Message *message, FILE *file);

#endif
