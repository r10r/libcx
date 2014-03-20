#include "message.h"

#define HEADER_LINE_FORMAT "%s: %s\n"
#define HEADER_LINE_FORMAT_NOV "%s:\n"
#define HEADER_LINE_FORMAT_LENGTH 3
#define ENVELOPE_SEPARATOR "\n"

static F_MessageEventHandler _parser_event_handler;

RagelParserState*
RagelParserState_new(unsigned int buffer_length)
{
	RagelParserState *state = malloc(sizeof(RagelParserState));

	state->res = 0;
	state->cs = 0;

	state->buffer = StringBuffer_new(buffer_length);
	state->buffer_position = NULL;
	state->buffer_end = NULL;
	state->eof = NULL;

	state->buffer_offset = 0;
	state->marker_start = 0;
	state->marker_length = 0;

	state->event = P_NONE;
	state->f_event_handler = _parser_event_handler;
	state->iterations = 0;

	return state;
}

void
RagelParserState_free(RagelParserState *state)
{
	StringBuffer_free(state->buffer);
	free(state);
}

static void
free_protocol_value(void *value)
{
	S_free((String*)value);
}

static void
free_header(void *value)
{
	StringPair_free((StringPair*)value);
}

Message *
Message_new(unsigned int buffer_length)
{
	Message *message = malloc(sizeof(Message));

	message->protocol_values = List_new();
	message->protocol_values->f_node_data_free = free_protocol_value;

	message->headers = List_new();
	message->headers->f_node_data_free = free_header;

	// FIXME using the same buffer length for body and parser
	message->body = StringBuffer_new(buffer_length);
	message->parser_state = RagelParserState_new(buffer_length);
	return message;
}

void
Message_free(Message *message)
{
	List_free(message->protocol_values);
	List_free(message->headers);
	StringBuffer_free(message->body);
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
	String *envelope = Message_envelope(message);
//	String_write(envelope, file);
	fprintf(file, "----------- end message\n");
}

#define ENVELOPE_DEFAULT_SIZE 1024

String*
Message_envelope(Message *message)
{
	String *envelope = String_init(NULL, ENVELOPE_DEFAULT_SIZE);

	// TODO append protocol line
	// TODO append headers
	return envelope;
}

ParseEvent
Message_parse_finish(Message *message)
{
	RagelParserState *state = message->parser_state;

	state->eof = state->buffer_end;
	ragel_parse_message(message);
	return state->event;
}

ParseEvent
Message_parse(Message *message)
{
	RagelParserState *state = message->parser_state;

	ragel_parse_message(message);
	return state->event;
}

ssize_t
Message_buffer_read(Message *message, int fd, size_t count)
{
	RagelParserState *state = message->parser_state;

	// see explanation in #Message_buffer_append
	ssize_t nread = StringBuffer_read_append(state->buffer, fd, count);

	state->buffer_position = &S_get(state->buffer->string, state->buffer_offset);
	state->buffer_end = &S_last(state->buffer->string);
	return nread;
}

ssize_t
Message_buffer_append(Message *message, const char *buf, size_t count)
{
	RagelParserState *state = message->parser_state;

	/*
	 * @optimize
	 * Buffer might be shifted up to last marker position or buffer offset
	 * (the one which is larger). both marker offset and position must be
	 * recalculated. Don't know whether this makes sense.
	 * Test whether shifting the buffer or double buffering (start a new
	 * buffer for each marker) is better.
	 */
	ssize_t nappended = StringBuffer_nappend(state->buffer, buf, count);

	/*
	 * string referenced by buffer might have been reallocated
	 * restore the buffer position
	 */
	state->buffer_position = &S_get(state->buffer->string, state->buffer_offset);
	state->buffer_end = &S_last(state->buffer->string);
	return nappended;
}

#define Marker_get(state) \
	& S_get(state->buffer->string, state->marker_start)

#define Marker_toS(state) \
	String_init(Marker_get(state), state->marker_length)

static void
_parser_event_handler(Message *message)
{
	RagelParserState *state = message->parser_state;

	XFLOG("Event %d, at index %zu [%c] \n",
	      state->event,
	      state->buffer_offset,
	      *state->buffer_position);

	switch (message->parser_state->event)
	{
	case P_NONE:
		// should not happen
		break;
	case P_ERROR_MESSAGE_MALFORMED:
		// do error handling here
		break;
	case P_PROTOCOL_VALUE:
		List_push(message->protocol_values, Marker_toS(state));
		break;
	case P_HEADER_NAME:
	{
		String *header_name = Marker_toS(state);
		StringPair *header = StringPair_init(header_name, NULL);
		List_push(message->headers, header);
		break;
	}
	case P_HEADER_VALUE:
	{
		StringPair *header = (StringPair*)message->headers->last->data;
		header->value = Marker_toS(state);
		break;
	}
	case P_BODY:
		state->marker_length++;
		StringBuffer_nappend(message->body, Marker_get(state), state->marker_length);
		break;
	}
}

int
Message_parse_file(Message *message, const char *file_path, size_t read_len)
{
	FILE *file = fopen(file_path, "r");

	XFASSERT(file, "file %s should exist\n", file_path);

	while (1)
	{
		ssize_t nread = Message_buffer_read(message, fileno(file), read_len);

		if (nread > 0)
		{
			Message_parse(message);
			continue;
		}
		else if (nread == 0)
		{
			Message_parse_finish(message);
			break;
		}
		else
		{
			XFLOG("error while reading file : %s : %s", file_path, strerror(errno));
			fclose(file);
			return -1;
		}
	}
	fclose(file);
	return 0;
}
