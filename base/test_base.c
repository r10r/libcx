#include "base/test.h"
#include "base.h"

struct foo
{
	int x;
};

struct bar
{
	int x;
	int y;
	const char* xxx;
	struct foo* foo;
};

static void
test_container_of()
{
	struct bar* bar = malloc(sizeof(struct bar));

	bar->x = 1;
	bar->y = 2;
	bar->xxx = "fooobar";
	bar->foo = malloc(sizeof(struct foo));

	struct bar* container;
	container = container_of(&bar->foo, struct bar, foo);
	TEST_ASSERT_EQUAL_PTR(bar, container);

	container = container_of(&bar->xxx, struct bar, xxx);
	TEST_ASSERT_EQUAL_PTR(bar, container);

	container = UC_container_of(&bar->foo, struct bar, foo);
	TEST_ASSERT_EQUAL_PTR(bar, container);

	const char** bla = &bar->xxx;
	container = container_of(bla, struct bar, xxx);
	TEST_ASSERT_EQUAL_PTR(bar, container);

	free(bar->foo);
	free(bar);
}

static void
test_clone()
{
	struct foo* foo = malloc(sizeof(struct foo));

	foo->x = 99;

	struct bar* bar = malloc(sizeof(struct bar));
	bar->x = 66;
	bar->y = 33;
	bar->xxx = "fooobar";
	bar->foo = foo;

	struct bar* cloned = clone(struct bar, bar);

	TEST_ASSERT_NOT_EQUAL(bar, cloned);
	TEST_ASSERT_EQUAL_INT(bar->x, cloned->x);
	TEST_ASSERT_EQUAL_INT(bar->y, cloned->y);
	TEST_ASSERT_EQUAL_STRING(bar->xxx, cloned->xxx);
	TEST_ASSERT_EQUAL_PTR(bar->foo, cloned->foo);

	free(foo);
	free(bar);
	free(cloned);
}

static void
test_unsigned_minus_signed()
{
	size_t s = 1024;
	int m = -24;

	TEST_ASSERT_EQUAL_INT(1000, (s - ((size_t)-m)));
}

struct node_t
{
	struct node_t* next;
	int num;
};

static void
test_EACH()
{
	struct node_t second = { .next = NULL, 2 };
	struct node_t first = { .next = &second, 1 };

	struct node_t* iterator = NULL;
	struct node_t* element = NULL;

	EACH(iterator, element, next)
	{
		; // noop
	}

	iterator = &first;
	element = NULL;
	int sum = 0;
	EACH(iterator, element, next)
	{
		sum += element->num;
	}

	TEST_ASSERT_EQUAL_INT(3, sum);
	TEST_ASSERT_EQUAL_INT(2, element->num);
}

static void
test_unsigned_arithmetic()
{
	unsigned int a = 5;
	unsigned int b = 7;

	int c = (int)(a - b);

	TEST_ASSERT_EQUAL_INT(-2, c);
}

int
main()
{
	TEST_BEGIN

	RUN(test_container_of);
	RUN(test_clone);
	RUN(test_unsigned_minus_signed);
	RUN(test_EACH);
	RUN(test_unsigned_arithmetic);

	TEST_END
}
