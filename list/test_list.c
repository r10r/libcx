#include "base/test.h"
#include "list.h"

static
int
data_strcmp(Node* node, void* data)
{
	return strcmp((char*)data, node->data);
}

static
void
node_free(void* data)
{
	cx_free(data);
}

static void
test_List_append()
{
	List* list = List_new();

	list->f_node_data_free = node_free;

	TEST_ASSERT_EQUAL_INT(0, List_append(list, strdup("node 1")));
	TEST_ASSERT_EQUAL_STRING("node 1", list->last->data);
	TEST_ASSERT_EQUAL_INT(1, List_append(list, strdup("node 2")));
	TEST_ASSERT_EQUAL_STRING("node 2", list->last->data);
	TEST_ASSERT_EQUAL_INT(2, List_append(list, strdup("node 3")));
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

	list->f_node_data_free = node_free;

	TEST_ASSERT_EQUAL_INT(0, List_push(list, strdup("node 1")));
	TEST_ASSERT_EQUAL_STRING("node 1", list->last->data);
	TEST_ASSERT_EQUAL_INT(1, List_push(list, strdup("node 2")));
	TEST_ASSERT_EQUAL_STRING("node 2", list->last->data);
	TEST_ASSERT_EQUAL_INT(2, List_push(list, strdup("node 3")));
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

	list->f_node_data_free = node_free;

	TEST_ASSERT_NULL(List_match(list, "node 1", data_strcmp));

	List_push(list, strdup("node 1"));
	List_push(list, strdup("node 2"));
	List_push(list, strdup("node 3"));

	TEST_ASSERT_NULL(List_match(list, "node", data_strcmp));

	TEST_ASSERT_EQUAL_STRING("node 2", List_match(list, "node 2", data_strcmp)->data);

	List_free(list);
}

static void
test_iterator(int index, Node* node)
{
	char buf[16];

	sprintf(buf, "node %d", index + 1);
	TEST_ASSERT_EQUAL_STRING(buf, node->data);
}

static void
test_List_each()
{
	List* list = List_new();

	list->f_node_data_free = node_free;

	List_each(list, test_iterator); /* test empty list */

	List_push(list, strdup("node 1"));
	List_push(list, strdup("node 2"));
	List_push(list, strdup("node 3"));

	List_each(list, test_iterator);

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
	TEST_END
}
