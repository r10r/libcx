#include "message_parser.h"

static void
event_handler(RagelParser *parser, int event);

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
	ragel_parser->f_event_handler = event_handler;
	ragel_parser->f_parse = message_fsm_parse;
	parser->f_parse_body = NULL;
	return parser;
}

Message*
MessageParser_free(MessageParser *parser)
{
	Message *message = parser->message;

	free(parser);
	return message;
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
	case P_NONE:
		// should not happen
		break;
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
		if (message_parser->f_parse_body)
		{
			// FIXME jump to body parser machine here
			message_parser->body_parser = clone(RagelParser, parser);
			message_parser->f_parse_body(message_parser->body_parser);
		}
		break;
	case P_BODY:
		parser->marker_length++;
		message->body = StringPointer_new(Marker_get(parser), parser->marker_length);
		break;
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
