#include "test/test.h"
#include "list.h"
#include <unistd.h> /* sleep */

NOSETUP
TRACE_INIT

/* test parameterization constants */
static int elements;
static int search_for;

int my_strcmp(Node *node, void *data)
{
	return strcmp(node->data.value, (char*)data);
}

void char_node_print(Node *node)
{
	printf("Node: %p\n", &node->data);
}

int hash_key_compare_key(Node *node, unsigned long key)
{
	return node->data.hash - key;
}

void free_string_node(Node *node)
{
	free(node->data.value);
}

void test_linked_list()
{
//	unsigned long x[ITEMS] = { 0 };

	// unitialized array does not take up any space
	// first write to array initializes it ?

	printf("LONG size %lu, data struct %lu\n", sizeof(unsigned long), sizeof(Data));

	TRACE_BEGIN_FMT("insert %d items\n", elements);

	int i;
	char buf[1024];

	List *list = List_new();
	list->free_node_cb = free_string_node;

	for (i = 0; i < elements; i++)
	{
		sprintf(buf, "foobar %d", i);
		List_append(list, strdup(buf));
	}
	TEST_ASSERT_EQUAL(elements, list->length);
	TRACE_END

	sprintf(buf, "foobar %d", search_for);
	TRACE_BEGIN_FMT("find item [%s] in %d items\n", buf, elements);
	for (i = 0; i < elements; i++)
	{
		Node *node = List_find(list, buf, my_strcmp);
		TEST_ASSERT_NOT_NULL(node);
//		TEST_ASSERT_EQUAL_STRING(search_value, node->data.value);
	}
	TRACE_END

//	TRACE_BEGIN("find one of " S(ITEMS) " hashed items");
//	unsigned long key = hash_djb2(search_value);
//	for(i = 0; i < 1000000; i++)
//	{
//		Node *node = List_find(list, key, hash_key_compare_key);
////		TEST_ASSERT_NOT_NULL(node);
////		TEST_ASSERT_EQUAL_STRING(search_value, node->data.value);
//	}
//	TRACE_END

// TODO measure memory usage
//	sleep(9999);

	TRACE_BEGIN_FMT("free %d elements\n", elements);
	List_destroy(list);

	TRACE_END
}

typedef struct my_foo_t
{
	unsigned long hash;
} MyFoo;

void test_malloc()
{
//	unsigned long foo[1000000];
//	MyFoo foo[1000000];
//	MyFoo *foo  = malloc(sizeof(MyFoo) * 1000000);
	sleep(9999);
}

int main()
{
	TEST_BEGIN

		elements = 100;

	search_for = 99;
	RUN(test_linked_list);

	elements = 1000;
	search_for = 500;
	RUN(test_linked_list);

	elements = 10000;
	search_for = 5000;
	RUN(test_linked_list);

//	elements = 100000;
//	search_for = 50000;
//	RUN(test_linked_list);

	TEST_END
}
