#include "message.h"

static void
free_protocol_value(void* value)
{
	S_free((String*)value);
}

static void
free_header(void* value)
{
	StringPair_free((StringPair*)value);
}

Message*
Message_new()
{
	Message* message = malloc(sizeof(Message));

	message->protocol_values = List_new();
	message->protocol_values->f_node_data_free = free_protocol_value;

	message->headers = List_new();
	message->headers->f_node_data_free = free_header;

	message->body = NULL;
	message->buffer = NULL;

	return message;
}

void
Message_free(Message* message)
{
	List_free(message->protocol_values);
	List_free(message->headers);
	StringBuffer_free(message->buffer);
	free(message->body);
	free(message);
}

void
Message_print_stats(Message* message, FILE* file)
{
	fprintf(file, "Message: %p\n", message);
	fprintf(file, "Counters: Protocol values [%ld], headers [%ld]\n",
		message->protocol_values->length, message->headers->length);
	fprintf(file, "----------- begin message\n");
	String* envelope = Message_envelope(message);
//	String_write(envelope, file);
	fprintf(file, "----------- end message\n");
}

#define ENVELOPE_DEFAULT_SIZE 1024

String*
Message_envelope(Message* message)
{
	String* envelope = String_init(NULL, ENVELOPE_DEFAULT_SIZE);

	// TODO append protocol line
	// TODO append headers
	return envelope;
}
