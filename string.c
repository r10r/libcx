//#include "libcx-base/xmalloc.h"
#include "string.h"
/* based on the redis sds.c (simple dynamic string) */

String
String_init(const void *value, size_t length)
{
	struct string_header_t *hdr;

	if (value)
		hdr =  malloc(String_size(length));
	else
		hdr =  calloc(String_size(length), 1);

	if (hdr == NULL)
		return NULL;         // OOM

	hdr->length = length;
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
	if (s == NULL)
		return;
	free(String_header(s));
}

String
String_grow(String s, size_t need)
{
	struct string_header_t *hdr, *new_hdr;
	size_t unused = String_unused(s);
	size_t length, new_length;

	if (unused > need)
		return s;
	length = String_length(s);
	hdr = String_header(s);

	new_length = length + need;
//  if (new_length < STRING_MAX_PREALLOC)
//      new_length *= 2;
//  else
//      new_length += STRING_MAX_PREALLOC;

	new_hdr = realloc(hdr, String_size(new_length));
	if (new_hdr == NULL)
		return NULL;
	new_hdr->length = new_length;
	new_hdr->unused = new_length - length;
	new_hdr->buf[new_length]  = '\0';

	return new_hdr->buf;
}

String
String_shrink(String s)
{
	return NULL; // NOT implemented
}

String
String_append(String a, String b)
{
	if (a == NULL)
		return NULL;
	if (b == NULL)
		return a;

	String c = String_grow(a, String_length(b));
	if (c == NULL)
		return NULL;
	memcpy(&c[String_length(a)], b, String_length(b));

	return c;
}

//Pair *
//StringPair_ndup(char *key, char *value)
//{
//	Pair *p = malloc(sizeof(Pair));
//	p->key = String_ndup(key);
//	p->value = String_ndup(value);
//	return p;
//}
//
//Pair *
//StringPair_map(char *key, int key_len, char *value, int value_len)
//{
//	Pair *p = malloc(sizeof(Pair));
//	p->key = String_const(key, key_len);
//	p->value = String_const(value, value_len);
//	return p;
//}
//
//void
//StringPair_free(Pair *p)
//{
//	String_free((String *) p->key);
//	String_free((String *) p->value);
//	free(p);
//}
