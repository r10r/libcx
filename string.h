#ifndef _STRING_H
#define _STRING_H

#include "stddef.h"     /* size_t */
#include <stdlib.h>     /* free */
#include <stdio.h>      /* printf */
#include <string.h>     /* memcpy */
#include "libcx-base/debug.h"

#define STRING_MAX_PREALLOC (1024 * 1024)

typedef char *String;
typedef struct pair_t Pair;

/* use char arrays instead of pointers to reduce overhead */
struct string_header_t
{
	unsigned int length;
	unsigned int unused;
	char buf[];
};

static inline struct string_header_t* String_header(const String s)
{
	return (struct string_header_t *)(s - sizeof(struct string_header_t));
}

static inline size_t String_length(const String s)
{
	return String_header(s)->length;
}

static inline size_t String_unused(const String s)
{
	return String_header(s)->unused;
}

static inline size_t String_size(int length)
{
	return sizeof(struct string_header_t) + sizeof(char) * length + 1;
}

struct pair_t
{
	void *key;
	void *value;
};

String
String_init(const void *value, size_t length);

String
String_new(const char *value);

void
String_free(String string);

String
String_grow(String s, size_t need);

String
String_append(String a, String b);

Pair *
StringPair_new(const char *key, const char *value);

void
StringPair_free(Pair *pair);

#endif
