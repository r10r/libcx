//#include "libcx-base/xmalloc.h"
#include "string.h"
/* based on the redis sds.c (simple dynamic string) */

String
String_init(const void *value, size_t length)
{
	struct string_header_t *hdr;

	if(value)
		hdr =  malloc(String_size(length));
	else
		hdr =  calloc(String_size(length), 1);

	if(hdr == NULL) return NULL; // OOM

	hdr->space = length;
	hdr->unused = 0;

	if (length > 0 && value)
		memcpy(hdr->buf, value, length);

	hdr->buf[length] =  '\0';
	return hdr->buf;
}

/* duplicate a null terminated string */
String
String_new(const char *value)
{
	size_t length = (value == NULL) ? 0 : strlen(value);
	return String_init(value, length);
}

void
String_free(String s)
{
	if (s == NULL) return;
	free(String_header(s));
}

String
String_grow(String s, size_t required_space)
{
  struct string_header_t *current_hdr, *new_hdr;
  size_t unused_space = String_unused(s);
  size_t current_length, new_length;
  if(unused_space >= required_space) return s;

  current_length = String_length(s);
  current_hdr = String_header(s);
  new_length = current_length + required_space;
//  if (new_length < STRING_MAX_PREALLOC)
//  	new_length *= 2;
//  else
//  	new_length += STRING_MAX_PREALLOC;

  new_hdr = realloc(current_hdr, String_size(new_length));
  if (new_hdr == NULL) return NULL;
  new_hdr->space = new_length;
  new_hdr->unused = new_length - current_length;
  new_hdr->buf[new_length]  = '\0';

  return new_hdr->buf;
}

String
String_shrink(String s)
{
	XASSERT("not yet implemented", 0);
	return NULL;
}

String
String_append(String a, String b)
{
	if (a == NULL) return NULL;
	if (b == NULL) return a;

	size_t a_length = String_length(a);

	String c = String_grow(a, String_length(b));
	if (c == NULL) return NULL;
	memcpy(&c[a_length], b, String_length(b));

	return c;
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
	String_free((String) p->key);
	String_free((String) p->value);
	free(p);
}
