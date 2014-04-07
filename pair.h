#ifndef _CX_STRING_PAIR_H
#define _CX_STRING_PAIR_H

#include "string.h"

typedef struct string_pair_t
{
	String* key;
	String* value;
} StringPair;

StringPair*
StringPair_init(String* key, String* value);

StringPair*
StringPair_new(const char* key, const char* value);

void
StringPair_free(StringPair* pair);

#endif
