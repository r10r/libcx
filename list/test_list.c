#include <libcx/base/test.h>
#include "list.h"

static void
test_List_append()
{
	List* list = List_new();

	TEST_ASSERT_EQUAL_INT(0, List_append(list, cx_strdup("node 1")));
	TEST_ASSERT_EQUAL_STRING("node 1", list->last->data);
	TEST_ASSERT_EQUAL_INT(1, List_append(list, cx_strdup("node 2")));
	TEST_ASSERT_EQUAL_STRING("node 2", list->last->data);
	TEST_ASSERT_EQUAL_INT(2, List_append(list, cx_strdup("node 3")));
	TEST_ASSERT_EQUAL_STRING("node 3", list->last->data);

	TEST_ASSERT_EQUAL_INT(3, list->length);

	TEST_ASSERT_EQUAL_STRING("node 1", list->first->data);
	TEST_ASSERT_EQUAL_STRING("node 2", list->first->next->data);
	TEST_ASSERT_EQUAL_STRING("node 2", list->last->previous->data);
	TEST_ASSERT_EQUAL_STRING("node 3", list->last->data);

	List_free(list);
}

static void
test_List_push()
{
	List* list = List_new();

	TEST_ASSERT_EQUAL_INT(0, List_push(list, cx_strdup("node 1")));
	TEST_ASSERT_EQUAL_STRING("node 1", list->last->data);
	TEST_ASSERT_EQUAL_INT(1, List_push(list, cx_strdup("node 2")));
	TEST_ASSERT_EQUAL_STRING("node 2", list->last->data);
	TEST_ASSERT_EQUAL_INT(2, List_push(list, cx_strdup("node 3")));
	TEST_ASSERT_EQUAL_STRING("node 3", list->last->data);

	TEST_ASSERT_EQUAL_INT(3, list->length);

	TEST_ASSERT_EQUAL_STRING("node 1", list->first->data);
	TEST_ASSERT_EQUAL_STRING("node 2", list->first->next->data);
	TEST_ASSERT_EQUAL_STRING("node 2", list->last->previous->data);
	TEST_ASSERT_EQUAL_STRING("node 3", list->last->data);

	List_free(list);
}

static void
test_List_match()
{
	List* list = List_new();

	list->f_node_data_compare = strcmp;

	Node* node = NULL;

	TEST_ASSERT_EQUAL_INT(-1, List_match_get_node(list, "node 1", &node));
	TEST_ASSERT_NULL(node);

	List_push(list, cx_strdup("node 1"));
	List_push(list, cx_strdup("node 2"));
	List_push(list, cx_strdup("node 3"));

	TEST_ASSERT_EQUAL_INT(-1, List_match_get_node(list, "node", &node));
	TEST_ASSERT_NULL(node);

	TEST_ASSERT_EQUAL(1, List_match_get_node(list, "node 2", &node));
	TEST_ASSERT_EQUAL_STRING("node 2", node->data);

	TEST_ASSERT_TRUE(LIST_CONTAINS(list, "node 1"));
	TEST_ASSERT_TRUE(LIST_CONTAINS(list, "node 2"));
	TEST_ASSERT_TRUE(LIST_CONTAINS(list, "node 3"));
	TEST_ASSERT_FALSE(LIST_CONTAINS(list, "node"));

	List_free(list);
}

static int
test_iterator(int index, Node* node, void* userdata)
{
	UNUSED(userdata);

	char buf[16];

	sprintf(buf, "node %d", index + 1);
	TEST_ASSERT_EQUAL_STRING(buf, node->data);
	return 1;
}

static void
test_List_each()
{
	List* list = List_new();

	List_each(list, test_iterator, NULL); /* test empty list */

	List_push(list, cx_strdup("node 1"));
	List_push(list, cx_strdup("node 2"));
	List_push(list, cx_strdup("node 3"));

	List_each(list, test_iterator, NULL);

	List_free(list);
}

static void
test_List_shift()
{
	List* list = List_new();

	List_push(list, "node 1");
	List_push(list, "node 2");
	List_push(list, "node 3");

	TEST_ASSERT_EQUAL_STRING("node 1", List_shift(list));
	TEST_ASSERT_EQUAL_STRING("node 2", list->first->data);
	TEST_ASSERT_EQUAL_STRING("node 3", list->last->data);
	TEST_ASSERT_EQUAL_INT(2, list->length);

	TEST_ASSERT_EQUAL_STRING("node 2", List_shift(list));
	TEST_ASSERT_EQUAL_STRING("node 3", list->first->data);
	TEST_ASSERT_EQUAL_STRING("node 3", list->last->data);
	TEST_ASSERT_EQUAL_INT(1, list->length);

	TEST_ASSERT_EQUAL_STRING("node 3", List_shift(list));
	TEST_ASSERT_EQUAL_INT(0, list->length);
	TEST_ASSERT_NULL(list->first);
	TEST_ASSERT_NULL(list->last);

	TEST_ASSERT_NULL(List_shift(list));
	TEST_ASSERT_EQUAL_INT(0, list->length);

	List_free(list);
}

static void
test_List_pop()
{
	List* list = List_new();

	List_push(list, "node 1");
	List_push(list, "node 2");
	List_push(list, "node 3");

	TEST_ASSERT_EQUAL_STRING("node 3", List_pop(list));
	TEST_ASSERT_EQUAL_STRING("node 1", list->first->data);
	TEST_ASSERT_EQUAL_STRING("node 2", list->last->data);
	TEST_ASSERT_EQUAL_INT(2, list->length);

	TEST_ASSERT_EQUAL_STRING("node 2", List_pop(list));
	TEST_ASSERT_EQUAL_STRING("node 1", list->first->data);
	TEST_ASSERT_EQUAL_STRING("node 1", list->last->data);
	TEST_ASSERT_EQUAL_INT(1, list->length);

	TEST_ASSERT_EQUAL_STRING("node 1", List_pop(list));
	TEST_ASSERT_EQUAL_INT(0, list->length);
	TEST_ASSERT_NULL(list->first);
	TEST_ASSERT_NULL(list->last);

	TEST_ASSERT_NULL(List_pop(list));
	TEST_ASSERT_EQUAL_INT(0, list->length);

	List_free(list);
}

static void
test_List_prepend()
{
	List* list = List_new();

	List_prepend(list, "node 1");
	List_prepend(list, "node 2");
	List_prepend(list, "node 3");

	TEST_ASSERT_EQUAL_INT(3, list->length);

	TEST_ASSERT_EQUAL_STRING("node 1", List_pop(list));
	TEST_ASSERT_EQUAL_STRING("node 2", List_pop(list));
	TEST_ASSERT_EQUAL_STRING("node 3", List_pop(list));

	TEST_ASSERT_EQUAL_INT(0, list->length);
	TEST_ASSERT_NULL(list->first);
	TEST_ASSERT_NULL(list->last);

	List_free(list);
}

static void
test_List_unshift()
{
	List* list = List_new();

	List_unshift(list, "node 1");
	List_unshift(list, "node 2");
	List_unshift(list, "node 3");

	TEST_ASSERT_EQUAL_INT(3, list->length);

	TEST_ASSERT_EQUAL_STRING("node 1", List_pop(list));
	TEST_ASSERT_EQUAL_STRING("node 2", List_pop(list));
	TEST_ASSERT_EQUAL_STRING("node 3", List_pop(list));

	TEST_ASSERT_EQUAL_INT(0, list->length);
	TEST_ASSERT_NULL(list->first);
	TEST_ASSERT_NULL(list->last);

	List_free(list);
}

static void
test_List_userdata_()
{
	List* list = List_new();

	TEST_ASSERT_NULL(List_userdata_get(list));
	List_userdata_set(list, "foobar");
	TEST_ASSERT_EQUAL_STRING("foobar", List_userdata_get(list));

	List_free(list);
}

static void
test_List_get()
{
	List* list = List_new();

	/* no need to free data, we are using constants */
	list->f_node_data_free = NULL;

	List_push(list, "node 1");
	List_push(list, "node 2");
	List_push(list, "node 3");

	TEST_ASSERT_EQUAL_STRING("node 1", List_get(list, 0));
	TEST_ASSERT_EQUAL_STRING("node 2", List_get(list, 1));
	TEST_ASSERT_EQUAL_STRING("node 3", List_get(list, 2));
	TEST_ASSERT_NULL(List_get(list, 3));

	List_free(list);
}

static void
test_List_free_empty_list()
{
	List* list = List_new();

	list->f_node_data_free = free;
	List_free(list);
}

static void
test_LIST_EACH_WITH_INDEX()
{
	List* list = List_new();

	List_push(list, cx_strdup("foo"));
	List_push(list, cx_strdup("bar"));
	List_push(list, cx_strdup("baz"));

	Node* head = list->first;
	Node* tail = NULL;
	unsigned int index = 0;
	LIST_EACH_WITH_INDEX(head, tail, index)
	{
		if (index == 0)
			TEST_ASSERT_EQUAL_STRING("foo", (char*)tail->data);
		if (index == 1)
			TEST_ASSERT_EQUAL_STRING("bar", (char*)tail->data);
		if (index == 2)
			TEST_ASSERT_EQUAL_STRING("baz", (char*)tail->data);

		TEST_ASSERT_TRUE(index < 3);
	}

	List_free(list);
}

static void
test_List_delete()
{
	List* list = List_new();

	List_push(list, cx_strdup("foo"));
	List_push(list, cx_strdup("bar"));
	List_push(list, cx_strdup("baz"));

	TEST_ASSERT_EQUAL_INT(3, list->length);
	List_delete(list, 1);
	TEST_ASSERT_EQUAL_INT(2, list->length);
	TEST_ASSERT_EQUAL_STRING("foo", (char*)List_get(list, 0));
	TEST_ASSERT_EQUAL_STRING("baz", (char*)List_get(list, 1));

	List_delete(list, 0);
	TEST_ASSERT_EQUAL_INT(1, list->length);
	TEST_ASSERT_EQUAL_STRING("baz", (char*)List_get(list, 0));

	List_delete(list, 0);
	TEST_ASSERT_EQUAL_INT(0, list->length);
	List_delete(list, 0);
	TEST_ASSERT_EQUAL_INT(0, list->length);

	List_free(list);
}

static const char* const foo = "foo";
static const char* const bar = "bar";
static const char* const baz = "baz";

static void
test_List_remove()
{
	List* list = List_new();

	list->f_node_data_free = NULL;

	List_push(list, foo);
	List_push(list, bar);
	List_push(list, baz);

	List_remove(list, bar);

	TEST_ASSERT_EQUAL_STRING(baz, (const char*)List_pop(list));
	TEST_ASSERT_EQUAL_STRING(foo, (const char*)List_pop(list));
	TEST_ASSERT_NULL(List_pop(list));

	List_free(list);
}

int
main()
{
	TEST_BEGIN
	RUN(test_List_append);
	RUN(test_List_push);
	RUN(test_List_match);
	RUN(test_List_each);
	RUN(test_List_shift);
	RUN(test_List_pop);
	RUN(test_List_prepend);
	RUN(test_List_unshift);
	RUN(test_List_userdata_);
	RUN(test_List_get);
	RUN(test_List_free_empty_list);
	RUN(test_LIST_EACH_WITH_INDEX);
	RUN(test_List_delete);
	RUN(test_List_remove);
	TEST_END
}
