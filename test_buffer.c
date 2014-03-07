#include "libcx-base/test.h"
#include "buffer.h"

NOSETUP

void test_Buffer()
{
	Buffer *buf = Buffer_new();
	Buffer_append(buf, "foo", 3);
	Buffer_append(buf, "bar", 3);
	TEST_ASSERT_EQUAL_STRING("foobar", buf->data);
	TEST_ASSERT_EQUAL_INT(6, buf->length);
	Buffer_free(buf);
}

int main()
{
	TEST_BEGIN

	RUN(test_Buffer);

	TEST_END
}
