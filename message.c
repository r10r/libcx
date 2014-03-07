#include "message.h"

#define HEADER_LINE_FORMAT "%s: %s\n"
#define HEADER_LINE_FORMAT_NOV "%s:\n"
#define HEADER_LINE_FORMAT_LENGTH 3
#define ENVELOPE_SEPARATOR "\n"

/* keep in sync with MethodType */
const char *METHOD_NAME[] =
{ "PUBLISH", "SUBSCRIBE", "UNSUBSCRIBE" };

MethodType
MethodType_of(const char *value)
{
	if (strcmp(value, METHOD_NAME[PUBLISH]) == 0)
		return PUBLISH;
	if (strcmp(value, METHOD_NAME[UNSUBSCRIBE]) == 0)
		return UNSUBSCRIBE;
	if (strcmp(value, METHOD_NAME[SUBSCRIBE]) == 0)
		return SUBSCRIBE;

	return UNDEFINED;
}

/* keep in sync with HeaderType */
const char *HEADER_NAME[] =
{ "Method", "Topic", "Sender" };

HeaderType
HeaderType_of(const char *value)
{
	if (strcmp(value, HEADER_NAME[METHOD]) == 0)
		return METHOD;
	if (strcmp(value, HEADER_NAME[TOPIC]) == 0)
		return TOPIC;
	if (strcmp(value, HEADER_NAME[SENDER]) == 0)
		return SENDER;

	return OTHER;
}

void
Message_detect_header_type(Message *message, Header *header)
{
	header->type = HeaderType_of(header->name);
	if (header->type == METHOD)
	{
		message->type = MethodType_of(header->value);
		message->method = header;
	}
	else if (header->type == TOPIC)
		message->topic = header;
	else if (header->type == SENDER)
		message->sender = header;
}

void
Envelope_new(Envelope *envelope)
{
	envelope->first = NULL;
	envelope->last = NULL;
	envelope->header_count = 0;
}

void ProtocolLine_new(ProtocolLine *protocol_line)
{
	protocol_line->value_count = 0;
	protocol_line->first_value = NULL;
	protocol_line->last_value = NULL;
}

void
Message_new(Message *message)
{
	ProtocolLine_new(&message->protocol_line);
	message->type = UNDEFINED;
	message->data = NULL;
	message->data_size = 0;
	Message_set_body(message, NULL, 0);
	Envelope_new(&message->envelope);
	message->method = NULL;
	message->topic = NULL;
	message->sender = NULL;
}

void
Header_set_name(Header *header, const char *name)
{
	if (name)
	{
		header->name = name;
		header->name_length = strlen(name);
	}
	else
	{
		header->name = NULL;
		header->name_length = 0;
	}
}

void
Header_set_value(Header *header, const char *value)
{
	if (value)
	{
		header->value = value;
		header->value_length = strlen(value);
	}
	else
	{
		header->value = NULL;
		header->value_length = 0;
	}
}

void
Header_new(Header *header, const char *name, const char *value)
{
	Header_set_name(header, name);
	Header_set_value(header, value);
	header->next = NULL;
	header->type = OTHER;
}

Header *
Envelope_add_header(Envelope *envelope, const char *name, const char *value)
{
	Header *header = malloc(sizeof(Header));

	Header_new(header, name, value);

	if (envelope->last)
	{
		envelope->last->next = header;
		envelope->last = header;
	}
	else
	{
		envelope->first = header;
		envelope->last = header;
	}
	envelope->header_count++;
	return header;
}

void
Message_free(Message *message)
{
	Header *header = message->envelope.first;

	while (header)
	{
		free(header);
		header = header->next;
	}
	free(message->data);
}

long
Envelope_length(Envelope *envelope)
{
	Header *header = envelope->first;
	long count = 0;

	while (header)
	{
		count += header->name_length
			 + header->value_length
			 + HEADER_LINE_FORMAT_LENGTH;
		header = header->next;
	}
	return count;
}

long
ProtocolLine_length(ProtocolLine *line)
{
	ProtocolValue *value = line->first_value;
	long count = 0;

	while (value)
	{
		count += value->length;
		value = value->next;
	}
	count += line->value_count; /* separators (blanks + newline) */
	return count;
}

long
Message_length(Message *msg)
{
	long envelope_length =  Envelope_length(&msg->envelope);
	long protocol_length = ProtocolLine_length(&msg->protocol_line);
	long body_length = 0;

	if (msg->body_length > 0)
	{
		/* if message body is not empty, we need to write the separator	 */
		body_length += strlen(ENVELOPE_SEPARATOR);
		body_length += msg->body_length;
	}
	return protocol_length + envelope_length + body_length;
}

long
Envelope_write_to_file(Envelope *envelope, FILE *file)
{
	Header *header = envelope->first;

	long count = 0;

	while (header)
	{
		if (header->value_length > 0)
			count += fprintf(file, HEADER_LINE_FORMAT, header->name, header->value);
		else
			count += fprintf(file, HEADER_LINE_FORMAT_NOV, header->name);
		header = header->next;
	}
	return count;
}

/* FIXME use snprintf and length/size parameters, don't rely on '\0' termination */
long
Envelope_write_to_buffer(Envelope *envelope, char *buf)
{
	Header *header = envelope->first;
	long count = 0;

	while (header)
	{
		if (header->value_length > 0)
			count += sprintf(&buf[count], HEADER_LINE_FORMAT, header->name, header->value);
		else
			count += sprintf(&buf[count], HEADER_LINE_FORMAT_NOV, header->name);
		header = header->next;
	}
	return count;
}

long
ProtocolLine_write_to_file(ProtocolLine *line, FILE *file)
{
	long count = 0;
	ProtocolValue *value = line->first_value;

	while (value)
	{
		if (count == 0)
			count += fprintf(file, "%s", value->data);
		else
			count += fprintf(file, " %s", value->data);

		value = value->next;
	}
	count += fprintf(file, "\n");
	return count;
}

long
ProtocolLine_write_to_buffer(ProtocolLine *line, char *buf)
{
	long count = 0;
	ProtocolValue *value = line->first_value;

	while (value)
	{
		if (count == 0)
			count += sprintf(&buf[count], "%s", value->data);
		else
			count += sprintf(&buf[count], " %s", value->data);

		value = value->next;
	}
	count += sprintf(&buf[count], "\n");
	return count;
}

void
Message_print_stats(Message *message, FILE *file)
{
	fprintf(file, "Message: %p\n", message);
	fprintf(file, "Counters: Protocol values [%d], headers [%d]\n",
		message->protocol_line.value_count, message->envelope.header_count);
	fprintf(file, "Lengths: protocol [%ld] envelope [%ld] body [%ld]\n",
		ProtocolLine_length(&message->protocol_line),
		Envelope_length(&message->envelope),
		message->body_length);
	fprintf(file, "----------- begin message\n");
	Message_write_to_file(message, file);
	fprintf(file, "----------- end message\n");
}

long
Message_write_to_file(Message *message, FILE *file)
{
	long count = 0;

	count += ProtocolLine_write_to_file(&message->protocol_line, file);
	count += Envelope_write_to_file(&message->envelope, file);

	if (message->body_length > 0)
	{
		count += fprintf(file, ENVELOPE_SEPARATOR);
		count += fprintf(file, "%s", message->body);
	}
	return count;
}

long
Message_write_to_buffer(Message *message, char *buf)
{
	long count = 0;

	count += ProtocolLine_write_to_buffer(&message->protocol_line, &buf[count]);
	count += Envelope_write_to_buffer(&message->envelope, &buf[count]);

	if (message->body_length > 0)
	{
		count += sprintf(&buf[count], ENVELOPE_SEPARATOR);
		count += sprintf(&buf[count], "%s", message->body);
	}
	return count;
}

Header *
Envelope_remove_header(Envelope *envelope, HeaderType type, char *name)
{
	// decrement header count
	// decrement envelope length
	return NULL;
}

/* TODO watch out for duplicates ! */
Header *
Message_set_header(Message *message, HeaderType type, const char *name, const char *value)
{
	Header *header = Envelope_add_header(&message->envelope, name, value);

	header->type = type;
	return header;
}

void
Message_set_topic(Message *message, const char *topic)
{
	Header *header = Message_set_header(message, TOPIC, HEADER_NAME[TOPIC], topic);

	message->topic = header;
}

void
Message_set_sender(Message *message, const char *sender)
{
	Header *header = Message_set_header(message, SENDER, HEADER_NAME[SENDER], sender);

	message->sender = header;
}

void
Message_set_method(Message *message, MethodType method)
{
	Header *header = Message_set_header(message, METHOD, HEADER_NAME[METHOD], METHOD_NAME[(int)method]);

	message->method = header;
	message->type = method;
}

/*
 * Set length to -1 to calculate length using strlen
 */
void
Message_set_body(Message *message, const char *body, long length)
{
	if (body == NULL || length == 0)
	{
		message->body = NULL;
		message->body_length = 0;
	}
	else
	{
		message->body = body;
		if (length > 0)
			message->body_length = length;
		else
			message->body_length = strlen(body);
	}
}

void
ProtocolValue_new(ProtocolValue *value, char *data, int length)
{
	value->data = data;
	value->length = length;
	value->next = NULL;
}

void
ProtocolLine_add_value(ProtocolLine *line, char* data, int length)
{
	/* FIXME use memory pool for allocation */
	ProtocolValue *value = malloc(sizeof(ProtocolValue));

	ProtocolValue_new(value, data, length);

	ProtocolValue *parent = line->last_value;
	if (parent)
	{
		parent->next = value;
		line->last_value = value;
	}
	else
	{
		line->first_value = value;
		line->last_value = value;
	}
	line->value_count++;
}
