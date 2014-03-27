#include "message_parser.h"

static void
event_handler(RagelParser *parser, int event);

static void
simple_body_parser(RagelParser *parser);

MessageParser *
MessageParser_new(size_t buffer_size)
{
	MessageParser *parser = malloc(sizeof(MessageParser));
	RagelParser *ragel_parser = (RagelParser*)parser;

	RagelParser_init(ragel_parser);
	parser->message = Message_new();

	/* setup parser buffer */
	StringBuffer *buffer = StringBuffer_new(buffer_size);
	parser->message->buffer = buffer;
	ragel_parser->buffer = buffer;

	/* setup event handlers */
	ragel_parser->f_event = event_handler;
	ragel_parser->f_parse = message_fsm_parse;
	parser->f_body_parse = simple_body_parser;
	parser->f_body_event = NULL;
	return parser;
}

Message*
MessageParser_free(MessageParser *parser)
{
	Message *message = parser->message;

	free(parser);
	return message;
}

/*  keep buffer, reset machine state and replace the machine and event handler */
void
MessageParser_parse_body(MessageParser *message_parser)
{
	// replace parser and event handler
	RagelParser *parser = (RagelParser*)message_parser;

	parser->f_parse = message_parser->f_body_parse;
	parser->f_event = message_parser->f_body_event;

	// reset machine state
	parser->res = 0;
	parser->cs = 0;
	parser->iterations = 0; // must be reset else initialization is skipped

	// process any remaining tokens
	size_t nunparsed =  RagelParser_unparsed(parser);
	if (nunparsed > 0)
	{
		printf("%zu unparsed tokens. calling body parser\n", nunparsed);
		RagelParser_parse(parser);
	}
}

static void
simple_body_parser(RagelParser *parser)
{
	RagelParser_update(parser); // update position

	// @optimize shift buffer up to body start
	if (RagelParser_eof(parser))
	{
		printf("EOF simple body parser\n");
		// marker length is not set since we do not count tokens in a FSM
		size_t marker_length = parser->buffer->string->length - parser->marker_start;
		StringPointer *body_pointer = StringPointer_new(Marker_get(parser), marker_length);
		((MessageParser*)parser)->message->body = body_pointer;
	}
	parser->iterations++;
}

static void
event_handler(RagelParser *parser, int event)
{
	MessageParser *message_parser = (MessageParser*)parser;
	Message *message = message_parser->message;

	XFLOG("Event %d, at index %zu [%c] \n",
	      event,
	      parser->buffer_offset,
	      *parser->buffer_position);

	switch (event)
	{
	case P_ERROR_MESSAGE_MALFORMED:
		// do error handling here
		break;
	case P_PROTOCOL_VALUE:
		List_push(message->protocol_values, Marker_toS(parser));
		break;
	case P_HEADER_NAME:
	{
		String *header_name = Marker_toS(parser);
		StringPair *header = StringPair_init(header_name, NULL);
		List_push(message->headers, header);
		break;
	}
	case P_HEADER_VALUE:
	{
		StringPair *header = (StringPair*)message->headers->last->data;
		header->value = Marker_toS(parser);
		break;
	}
	case P_BODY_START:
		MessageParser_parse_body(message_parser);
		return;
	}
}

#define CHUNK_SIZE 1024

Message *
MessageParser_fread(const char *path)
{
	MessageParser *parser = MessageParser_new(CHUNK_SIZE);

	RagelParser_parse_file((RagelParser*)parser, path, CHUNK_SIZE);

	// do something useful with the message
	Message *message = parser->message;
	MessageParser_free(parser);
	return message;
}
