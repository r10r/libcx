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
	Message* message = calloc(1, sizeof(Message));

	message->protocol_values = List_new();
	message->protocol_values->f_node_data_free = free_protocol_value;

	message->headers = List_new();
	message->headers->f_node_data_free = free_header;

	return message;
}

void
Message_free(Message* message)
{
	List_free(message->protocol_values);
	List_free(message->headers);
	if (!message->keep_buffer)
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

ssize_t
Message_write(Message* message, int fd)
{
	StringBuffer* buf = StringBuffer_new(2048);

	Message_print(message, buf);
	ssize_t written = StringBuffer_write(buf, fd);
	StringBuffer_free(buf);
	return written;
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

const char*
Message_get_protocol_value(Message* message, unsigned int index)
{
	String* value = (String*)List_get(message->protocol_values, index);

	if (value)
		return value->value;
	else
		return NULL;
}

int
Message_protocol_value_equals(Message* message, unsigned int index, const char* expected, int ignorecase)
{
	const char* actual = Message_get_protocol_value(message, index);

	if (actual)
	{
		if (ignorecase)
			return strcasecmp(expected, actual) == 0;
		else
			return strcmp(expected, actual) == 0;
	}
	return 0;
}

void
Message_set_header(Message* message, const char* key, const char* value)
{
	StringPair* pair = Message_get_header(message, key);

	if (pair)
	{
		/* remove old value, set new value */
		S_free(pair->value);
		pair->value = S_dup(value);
	}
	else
		List_push(message->headers, StringPair_new(key, value));
}

StringPair*
Message_get_header(Message* message, const char* key)
{
	Node* head = message->headers->first;
	Node* node;

	LIST_EACH(head, node)
	{
		StringPair* header = (StringPair*)node->data;

		assert(header);
		if (strcmp(key, header->key->value) == 0)
			return header;
	}
	return NULL;
}

int
Message_link_header_value(Message* message, const char* name, const char** const destination)
{
	assert(destination);
	StringPair* header = Message_get_header(message, name);
	if (header)
	{
		*destination = header->value->value;
		return 1;
	}
	else
		return 0;
}

int
Message_header_value_equals(Message* message, const char* name, const char* value, int ignorecase)
{
	StringPair* header = Message_get_header(message, name);

	if (header)
	{
		if (ignorecase)
			return strcasecmp(value, header->value->value) == 0;
		else
			return strcmp(value, header->value->value) == 0;
	}

	return 0;
}
