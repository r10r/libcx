#include "string.h"
/* based on the redis sds.c (simple dynamic string) */

String
String_init(const void *value, unsigned int length)
{
	// TODO check if maximum length is exceeded
	struct string_header_t *hdr;

	if (value)
	{
		hdr =  malloc(String_size(length));
		memcpy(hdr->buf, value, length);
		hdr->unused = 0;
		hdr->buf[length] =  '\0';
	}
	else
	{
		hdr =  calloc(String_size(length), sizeof(char));
		hdr->unused = length;
		hdr->buf[0] =  '\0';

	}

	hdr->available = length;
	return hdr->buf;
}

/* duplicate a null terminated string */
String
String_new(const char *value)
{
	// asssert strlen(value) < UINT32_MAX
	unsigned int length = (value == NULL) ? 0 : (unsigned int)strlen(value);

	return String_init(value, length);
}

void
String_free(String s)
{
	if (s == NULL)
		return;
	free(String_header(s));
}

String
String_grow(String s, unsigned int required_size)
{
	// TODO check if maximum length is exceeded
	struct string_header_t *current_hdr, *new_hdr;
	unsigned int unused_space = String_unused(s);
	unsigned int current_length, new_length;

	if (unused_space >= required_size)
		return s;

	current_length = String_length(s);
	current_hdr = String_header(s);
	new_length = current_length + required_size;
//  if (new_length < STRING_MAX_PREALLOC)
//      new_length *= 2;
//  else
//      new_length += STRING_MAX_PREALLOC;

	new_hdr = realloc(current_hdr, String_size(new_length));
	if (new_hdr == NULL)
		return NULL;
	new_hdr->available = new_length;
	new_hdr->unused = new_length - current_length;
	new_hdr->buf[new_length]  = '\0';

	return new_hdr->buf;
}

//String
//String_shrink(String s)
//{
//	XASSERT("not yet implemented", 0);
//	return NULL;
//}

static inline String
_append(String a, size_t a_length, const char *b, unsigned int b_length)
{
	String c = String_grow(a, b_length);

	if (c == NULL)
		return NULL;
	memcpy(&c[a_length], b, b_length);
	struct string_header_t *hdr = String_header(c);
	hdr->unused -= b_length;
	return c;
}

String
String_append_array(String a, const char *b, unsigned int b_length)
{
	if (a == NULL)
		return NULL;
	if (b == NULL || b_length == 0)
		return a;
	return _append(a, String_length(a), b, b_length);
}

String
String_append_constant(String a, const char *b)
{
	if (a == NULL)
		return NULL;
	if (b == NULL || strlen(b) == 0)
		return a;
	return _append(a, String_length(a), b, (unsigned int)strlen(b));
}

String
String_append(String a, String b)
{
	if (a == NULL)
		return NULL;
	if (b == NULL || String_length(b) == 0)
		return a;
	return _append(a, String_length(a), b, String_length(b));
}

Pair *
StringPair_init(String key, String value)
{
	Pair *p = malloc(sizeof(Pair));

	p->key = key;
	p->value = value;
	return p;
}

Pair *
StringPair_new(const char *key, const char *value)
{
	return StringPair_init(String_new(key), String_new(value));
}

void
StringPair_free(Pair *p)
{
	String_free((String)p->key);
	String_free((String)p->value);
	free(p);
}

size_t
String_write(String s, FILE *file)
{
	return fwrite(s, sizeof(char), String_length(s), file);
}

String
String_append_stream(String s, FILE *file, unsigned int length)
{
	unsigned int required_size = length * sizeof(char);
	String buf = String_grow(s, required_size);

	struct string_header_t *hdr = String_header(buf);
	/* we don't read more than required_size chars/bytes */
	unsigned int read = (unsigned int )fread(&buf[String_length(buf)], 1, required_size, file);

	hdr->unused = required_size - read;
	return buf;
}

ssize_t
String_fread(String s, FILE *file)
{
	return String_read(s, fileno(file));
}

ssize_t
String_read(String s, int fd)
{
	unsigned int len = String_available(s);
	unsigned int len_read =  (unsigned int)read(fd, &s[0], len);

	s[len_read] = '\0';
	String_header(s)->unused = len - len_read;
	return len_read;
}

void
String_clear(String s)
{
	struct string_header_t *hdr = String_header(s);

	hdr->unused = hdr->available;
	s[0] = '\0';
}

void
String_shift(String s, int unsigned count)
{
	if (String_length(s) < count)
		String_clear(s);
	else
	{
		memcpy(&s[0], &s[count], count);
		String_header(s)->unused += count;
		s[String_length(s)] = '\0';
	}
}
