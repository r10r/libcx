#ifndef _STRING_H
#define _STRING_H

#include "stddef.h"     /* size_t */
#include <stdlib.h>     /* free */
#include <string.h>     /* memcpy */
#include <unistd.h>     /* write */
#include "libcx-base/debug.h"

#define STRING_MAX_PREALLOC (1024 * 1024)
//#define STRING_MAX_LENGTH (UINT32_MAX - sizeof(struct string_header_t))
// TODO define custom type e.g string_size_t ?

typedef char *String;
typedef struct pair_t Pair;

/* use char arrays instead of pointers to reduce overhead */
// FIXME string alignment is 4 bytes instead of 1 when it is casted to the header
struct string_header_t
{
	unsigned int available;
	unsigned int unused;
	char buf[];
};

static inline struct string_header_t* String_header(const String s)
{
	return (struct string_header_t *)(s - sizeof(struct string_header_t));
}

static inline unsigned int String_space(const String s)
{
	return String_header(s)->available;
}

static inline unsigned int String_unused(const String s)
{
	return String_header(s)->unused;
}

/* the string size excluding the '\0' terminator */
static inline unsigned int String_length(const String s)
{
	return String_space(s) - String_unused(s);
}

static inline unsigned int String_size(unsigned int length)
{
	return (unsigned int)sizeof(struct string_header_t) + (sizeof(char) * (length + 1 /* '\0' */));
}

struct pair_t
{
	void *key;
	void *value;
};

String
String_init(const void *value, unsigned int length);

String
String_new(const char *value);

void
String_free(String string);

String
String_grow(String s, unsigned int need);

String
String_shrink(String s);

String
String_append(String a, String b);

String
String_append_array(String a, const char *b, unsigned int b_length);

String
String_append_constant(String a, const char *b);

String
String_append_stream(String s, FILE *file, unsigned int length);

Pair *
StringPair_init(String key, String value);

Pair *
StringPair_new(const char *key, const char *value);

void
StringPair_free(Pair *pair);

size_t
String_write(String s, FILE *file);

#endif
