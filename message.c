#include "message.h"

#define HEADER_LINE_FORMAT "%s: %s\n"
#define HEADER_LINE_FORMAT_NOV "%s:\n"
#define HEADER_LINE_FORMAT_LENGTH 3
#define ENVELOPE_SEPARATOR "\n"

static F_MessageEventHandler _parser_event_handler;

RagelParserState*
RagelParserState_new()
{
	RagelParserState *state = malloc(sizeof(RagelParserState));

	state->res = 0;
	state->cs = 0;

	state->buffer_position = NULL;
	state->buffer_end = NULL;
	state->buffer_offset = 0;
	state->eof = NULL;

	state->marker = NULL;
	state->marker_offset = 0;

	state->event = P_NEW;
	state->f_event_handler = _parser_event_handler;
	return state;
}

void
RagelParserState_free(RagelParserState *state)
{
	free(state);
}

static void
free_protocol_value(void *value)
{
	String_free((String)value);
}

static void
free_header(void *value)
{
	StringPair_free((Pair*)value);
}

Message *
Message_new(size_t body_size)
{
	Message *message = malloc(sizeof(Message));

	message->protocol_values = List_new();
	message->protocol_values->f_node_data_free = free_protocol_value;

	message->headers = List_new();
	message->headers->f_node_data_free = free_header;

	message->buffer = String_init(NULL, body_size);
	message->body = String_new(NULL);

	message->parser_state = RagelParserState_new();
	return message;
}

void
Message_free(Message *message)
{
	List_free(message->protocol_values);
	List_free(message->headers);
	String_free(message->buffer);
	String_free(message->body);
	RagelParserState_free(message->parser_state);
	free(message);
}

void
Message_print_stats(Message *message, FILE *file)
{
	fprintf(file, "Message: %p\n", message);
	fprintf(file, "Counters: Protocol values [%ld], headers [%ld]\n",
		message->protocol_values->length, message->headers->length);
	fprintf(file, "----------- begin message\n");
	String envelope = Message_envelope(message);
	String_write(envelope, file);
	String_write(message->buffer, file);
	fprintf(file, "----------- end message\n");
}

#define ENVELOPE_DEFAULT_SIZE 1024

String
Message_envelope(Message *message)
{
	String envelope = String_init(NULL, ENVELOPE_DEFAULT_SIZE);

	// TODO append protocol line
	// TODO append headers
	return envelope;
}

long
Message_fwrite(Message *message, FILE *file, bool with_body)
{
	return 0L;
}

extern void ragel_parse_message(Message *message);

ParseEvent
Message_parse_finish(Message *message)
{
	RagelParserState *s = message->parser_state;

	// buffer might be reallocated so we start at the offset
	// TODO move to String_index
	s->buffer_position = &message->buffer[s->buffer_offset];
	s->buffer_end = String_last(message->buffer);
	s->eof = s->buffer_end;
	ragel_parse_message(message);
	return s->event;
}

ParseEvent
Message_parse(Message *message)
{
	RagelParserState *s = message->parser_state;

	// buffer might be reallocated so we start at the offset
	// TODO move to String_index
	s->buffer_position = &message->buffer[s->buffer_offset];
	s->buffer_end = String_last(message->buffer);
	s->eof = NULL;
	ragel_parse_message(message);
	return s->event;
}

static void
_parser_event_handler(Message *message)
{
	RagelParserState *state = message->parser_state;

	XFLOG("Event %d, at index %ld [%c] \n", state->event, state->buffer_offset, *state->buffer_position);

	switch (message->parser_state->event)
	{
	case P_PROTOCOL_VALUE:
		List_push(message->protocol_values, String_init(state->marker, state->marker_offset));
		break;
	case P_HEADER_NAME:
	{
		String header_name = String_init(state->marker, state->marker_offset);
		Pair *header = StringPair_init(header_name, NULL);
		List_push(message->headers, header);
		break;
	}
	case P_HEADER_VALUE:
	{
		Pair *header = (Pair*)message->headers->last->data;
		header->value = String_init(state->marker, state->marker_offset);
		break;
	}
	case P_BODY:
		message->body = String_append_array(message->body, state->marker, state->marker_offset + 1);
		break;
	}
}

