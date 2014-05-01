#ifndef _CX_MESSAGE_PARSER_H
#define _CX_MESSAGE_PARSER_H

#include "message.h"
#include "base/base.h"

typedef enum parse_event_t
{
	P_PROTOCOL_VALUE,
	P_HEADER_NAME,
	P_HEADER_VALUE,
	P_BODY_START,
	P_ERROR_MESSAGE_MALFORMED
} ParseEvent;

typedef struct message_parser_t
{
	RagelParser header_parser;
	F_ParseHandler* f_body_parse;
	F_EventHandler* f_body_event;
	Message* message;
} MessageParser;

/* implemented in message_fsm.c */
extern void
message_fsm_parse(RagelParser* parser);

MessageParser*
MessageParser_from_buf(StringBuffer* buffer);

#define MessageParser_new(buffer_size) \
	MessageParser_from_buf(StringBuffer_new(buffer_size));

Message*
MessageParser_free(MessageParser* parser);

void
MessageParser_parse_body(MessageParser* message_parser);

Message*
MessageParser_fread(const char* file_path);

#endif
