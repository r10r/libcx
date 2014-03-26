#include "message.h"

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
Message_new()
{
	Message *message = malloc(sizeof(Message));

	message->protocol_values = List_new();
	message->protocol_values->f_node_data_free = free_protocol_value;

	message->headers = List_new();
	message->headers->f_node_data_free = free_header;

	message->body = NULL;
	message->buffer = NULL;

	return message;
}

void
Message_free(Message *message)
{
	List_free(message->protocol_values);
	List_free(message->headers);
	StringBuffer_free(message->buffer);
	free(message->body);
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

static void
event_handler(RagelParser *parser, int event);

RagelParser *
MessageParser_new(size_t buffer_size)
{
	RagelParser *parser = RagelParser_new();
	Message *message = Message_new();

	message->buffer = StringBuffer_new(buffer_size);

	parser->userdata = message;
	parser->f_event_handler = event_handler;
	parser->buffer = message->buffer;

	return parser;
}

static void
event_handler(RagelParser *parser, int event)
{
	XFLOG("Event %d, at index %zu [%c] \n",
	      event,
	      parser->buffer_offset,
	      *parser->buffer_position);

	Message *message = (Message*)parser->userdata;

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
	case P_BODY:
		parser->marker_length++;
		message->body = StringPointer_new(Marker_get(parser), parser->marker_length);
		break;
	}
}

#define CHUNK_SIZE 1024

Message *
Message_fread(const char *path)
{
	RagelParser *parser = MessageParser_new(CHUNK_SIZE);
	Message *message = (Message*)RagelParser_parse_file(parser, path, CHUNK_SIZE);

	// do something useful with the message
	RagelParser_free(parser);
	return message;
}
