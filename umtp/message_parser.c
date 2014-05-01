#include "message_parser.h"

static void
event_handler(RagelParser* parser, int event);

static void
simple_body_parser(RagelParser* parser);

MessageParser*
MessageParser_from_buf(StringBuffer* buffer, int keep_buffer)
{
	MessageParser* parser = malloc(sizeof(MessageParser));
	RagelParser* ragel_parser = (RagelParser*)parser;

	RagelParser_init(ragel_parser);
	parser->message = Message_new();

	parser->message->buffer = buffer;
	parser->message->keep_buffer = keep_buffer;
	ragel_parser->buffer = buffer;

	// set buffer pointer
	ragel_parser->buffer_position = buffer->string->value;
	ragel_parser->buffer_end = ragel_parser->buffer_position;

	/* setup event handlers */
	ragel_parser->f_event = event_handler;
	ragel_parser->f_parse = message_fsm_parse;
	parser->f_body_parse = simple_body_parser;
	parser->f_body_event = NULL;
	return parser;
}

Message*
MessageParser_free(MessageParser* parser)
{
	Message* message = parser->message;

	free(parser);
	return message;
}

/*  keep buffer, reset machine state and replace the machine and event handler */
void
MessageParser_parse_body(MessageParser* message_parser)
{
	// replace parser and event handler
	RagelParser* parser = (RagelParser*)message_parser;

	parser->f_parse = message_parser->f_body_parse;
	parser->f_event = message_parser->f_body_event;
}

static void
simple_body_parser(RagelParser* parser)
{
	RagelParser_update(parser); // update position

	// @optimize shift buffer up to body start
	if (RagelParser_eof(parser))
	{
		size_t nunparsed = RagelParser_unparsed(parser);
		assert(nunparsed > 0);

		/* save remaining tokens in body */
		StringPointer* body_pointer = StringPointer_new(Marker_get(parser), nunparsed);
		((MessageParser*)parser)->message->body = body_pointer;
		/* advance parser pointer to the end */
		parser->buffer_position = parser->buffer_end;
	}

	parser->iterations++;
}

static void
event_handler(RagelParser* parser, int event)
{
	MessageParser* message_parser = (MessageParser*)parser;
	Message* message = message_parser->message;


	XFDBG("Event %d, at index %zu [%c]",
	      event,
	      /* buffer already position points to next token */
	      parser->buffer_offset - 1,
	      *(parser->buffer_position - 1));

	switch (event)
	{
	case P_ERROR_MESSAGE_MALFORMED:
		// do error handling here
		break;
	case P_PROTOCOL_VALUE:
		// use Message_* functions ?
		List_push(message->protocol_values, Marker_toS(parser));
		break;
	case P_HEADER_NAME:
	{
		// use Message_* functions ?
		String* header_name = Marker_toS(parser);
		StringPair* header = StringPair_init(header_name, NULL);
		List_push(message->headers, header);
		break;
	}
	case P_HEADER_VALUE:
	{
		// use Message_* functions ?
		StringPair* header = (StringPair*)message->headers->last->data;
		header->value = Marker_toS(parser);
		break;
	}
	case P_BODY_START:
		// TODO mark header as finished here !!!
		MessageParser_parse_body(message_parser);
		return;
	}
}

#define CHUNK_SIZE 1024

MessageParser*
MessageParser_fread(const char* path)
{
	MessageParser* parser = MessageParser_new(CHUNK_SIZE);

	RagelParser_parse_file((RagelParser*)parser, path, CHUNK_SIZE);
	return parser;
}
