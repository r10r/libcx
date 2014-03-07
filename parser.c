#include "parser.h"

static const char * const PARSE_EVENT_STRING[] =
{ "PROTOCOL_VALUE", "HEADER_NAME", "HEADER_VALUE", "BODY" };

static void
parse_callback(MachineState *state, Message *message, ParseEvent event)
{
	if (state->debug)
	{
		XFLOG("eof: %p, pe: %p, p: %p\n", state->eof, state->pe, state->p);
		XFLOG("Event %s, at index %ld [%c] \n",
		       PARSE_EVENT_STRING[(int)event], state->token_count, *state->p);
	}

	switch (event)
	{
	case P_PROTOCOL_VALUE:
		*state->p = '\0';
		ProtocolLine_add_value(&message->protocol_line, state->mark, state->mark_offset);
		break;
	case P_HEADER_NAME:
		*state->p = '\0';
		Envelope_add_header(&(message->envelope), state->mark, NULL);
		break;
	case P_HEADER_VALUE:
	{
		/*
		 * if we reached EOF, setting the nullterminator would overwrite
		 * the last input character, so we have to check that
		 */
		int lastchar = ((state->eof == state->pe) && (state->pe == state->p));
		if (!lastchar)
			*state->p = '\0';
		Header *header = message->envelope.last;
		Header_set_value(header, state->mark);
		Message_detect_header_type(message, header);
		break;
	}
	case P_BODY:
		/* length = offset + 1, because offset begins with 0 */
		Message_set_body(message, state->mark, state->mark_offset);
		break;
	}
}

void
MachineState_new(MachineState *state)
{
	state->res = 0;
	state->cs = 0;
	state->pe = NULL;
	state->mark = NULL;
	state->eof = NULL;
	state->cb = NULL;
	state->token_count = 0;
	state->mark_offset = 0;
	state->debug = false;
}

int
Message_parse(Message *message, char *buf, size_t size)
{
	if (size == 0 || buf == NULL)
	{
		XLOG("No input\n");
		return 0;
	}
	else
	{
		XFLOG("input size : %zu\n", size);
		MachineState state;

		MachineState_new(&state);
#ifdef TRACE
		state.debug = true;
#endif
		state.cb = parse_callback;
		Message_new(message);
		// using malloc is faster than strdup -> proof
		// message->data = strdup(buf);
//		message->data = malloc(size + 1); /* terminating /0 */
//		message->data[size] = '\0';
//		memcpy(message->data, buf, size);
		message->data = strndup(buf, size);
		message->data_size = size;
		return ragel_parse_message(&state, message, true);
	}
}
