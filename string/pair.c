#include "pair.h"

StringPair*
StringPair_init(String* key, String* value)
{
	StringPair* p = cx_alloc(sizeof(StringPair));

	p->key = key;
	p->value = value;
	return p;
}

StringPair*
StringPair_new(const char* key, const char* value)
{
	return StringPair_init(S_dup(key), S_dup(value));
}

void
StringPair_free(StringPair* p)
{
	S_free(p->key);
	S_free(p->value);
	cx_free(p);
}
