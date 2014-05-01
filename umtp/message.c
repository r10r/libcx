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
Message_print(Message* message, StringBuffer* buffer)
{
	Message_print_envelope(message, buffer);
	StringBuffer_cat(buffer, MESSAGE_LF);
	if (message->body)
		StringBuffer_ncat(buffer, message->body->value, message->body->length);
}

void
Message_print_envelope(Message* message, StringBuffer* buffer)
{
	/* TODO add something like StringBuffer_join(buffer, separator, values[], iterator_func) */
	unsigned int values = (unsigned int)message->protocol_values->length;
	unsigned int nvalue;

	for (nvalue = 0; nvalue < values; nvalue++)
	{
		const char* value = ((String*)List_get(message->protocol_values, nvalue))->value;
		if (nvalue < values - 1)
			StringBuffer_aprintf(buffer, "%s ", value);
		else
			StringBuffer_aprintf(buffer, "%s" MESSAGE_LF, value);
	}

	Node* head = message->headers->first;
	Node* header;
	LIST_EACH(head, header)
	{
		StringPair* hdr = (StringPair*)header->data;

		StringBuffer_aprintf(buffer, "%s: %s" MESSAGE_LF, hdr->key->value, hdr->value->value);
	}
}
