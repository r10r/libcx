#ifndef _PARSER_H
#define _PARSER_H

#include "stdio.h"
#include "message.h"
#include "libcx-base/debug.h"

typedef enum parse_event_t
{
	P_PROTOCOL_VALUE, P_HEADER_NAME, P_HEADER_VALUE, P_BODY
} ParseEvent;

typedef struct state_machine_t
{
	char *p;
	char *pe;
	char *eof;
	int res;
	int cs;
	char *mark;
	int mark_offset;
	long token_count;
	bool debug;
	void
	(*cb)(struct state_machine_t *state, Message *message, ParseEvent event);
} MachineState;

int
Message_parse(Message *message, char *buf, size_t size);

int
ragel_parse_message(MachineState *state, Message *message, bool eof);

#endif
