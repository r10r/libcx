#include "libcx-base/test.h"
#include "base.h"

NOSETUP

struct foo
{
	int x;
};

struct bar
{
	int x;
	int y;
	const char *xxx;
	struct foo *foo;
};

static void test_container_of()
{
	struct bar *bar = malloc(sizeof(struct bar));

	bar->x = 1;
	bar->y = 2;
	bar->xxx = "fooobar";
	bar->foo = malloc(sizeof(struct foo));

	struct bar *container;
	container = container_of(&bar->foo, struct bar, foo);
	TEST_ASSERT_EQUAL_PTR(bar, container);

	container = UC_container_of(&bar->foo, struct bar, foo);
	TEST_ASSERT_EQUAL_PTR(bar, container);

	free(bar->foo);
	free(bar);
}

int main()
{
	TEST_BEGIN

	RUN(test_container_of);

	TEST_END
}
