#include "list.h"

Node*
Node_new()
{
	Node* node = cx_alloc(sizeof(Node));

	return node;
}

void
Node_free(Node* node, F_NodeDataFree* f_node_data_free)
{
	if (f_node_data_free)
		f_node_data_free(node->data);
	cx_free(node);
}

static inline void
Node_data_free_simple(void* data)
{
	cx_free(data);
}

static inline int
Node_data_compare_address(const char* node_data, const char* key)
{
	return (int)(key - node_data);
}

void
List_init(List* list)
{
	list->f_node_data_free = &Node_data_free_simple;
	list->f_node_data_compare = &Node_data_compare_address;
}

List*
List_new()
{
	List* list = cx_alloc(sizeof(List));

	List_init(list);
	return list;
}

void
List_free_members(List* list)
{
	Node* next = list->first;

	while (next)
	{
		Node* cur = next;
		next = cur->next;
		Node_free(cur, list->f_node_data_free);
	}
}

void
List_free(List* list)
{
	if (list)
	{
		List_free_members(list);
		cx_free(list);
	}
}

static inline long
_List_push(List* list, void* data)
{
	Node* new = Node_new();

	new->data = data;

	Node* parent = list->last;
	if (parent)
	{
		parent->next = new;
		new->previous = parent;
		list->last = new;
	}
	else
	{
		list->first = new;
		list->last = new;
	}
	long length = list->length++;
	return length;
}

long
List_append(List* list, void* data)
{
	return _List_push(list, data);
}

long
List_push(List* list, void* data)
{
	return _List_push(list, data);
}

/* @return the node index or -1 if the node was not found
 * @argument ptr points to the node if return value is > -1
 */
long
List_match_get_node(List* list, const char* key, Node** node_ptr)
{
	if (list->length == 0)
		return -1;

	long idx = 0;
	Node* node = list->first;
	while (node)
	{
		if (list->f_node_data_compare(node->data, key) == 0)
		{
			if (node_ptr)
				*node_ptr = node;
			return idx;
		}
		else
		{
			node = node->next;
			idx++;
		}
	}
	return -1;
}

long
List_match_get_data(List* list, const char* key, void** data_ptr)
{
	Node* node = NULL;
	long node_idx = List_match_get_node(list, key, &node);

	if (node)
		*data_ptr = node->data;

	return node_idx;
}

void
List_each(List* list, F_NodeIterator* f_node_iterator, void* userdata)
{
	if (list->length == 0)
		return;
	Node* node = list->first;

	int index = 0;
	while (node && f_node_iterator(index, node, userdata))
	{
		node = node->next;
		index++;
	}
}

void*
List_shift(List* list)
{
	if (list->length == 0)
		return NULL;
	Node* node = list->first;
	void* data = NULL;

	if (node)
	{
		list->first = node->next;
		if (list->first)
			list->first->previous = NULL;
		else
			list->last = NULL;

		list->length--;

		data = node->data;
		Node_free(node, NULL);
	}
	return data;
}

void*
List_pop(List* list)
{
	if (list->length == 0)
		return NULL;

	Node* node = list->last;
	void* data = NULL;
	if (node)
	{
		list->last = node->previous;
		if (list->last)
			list->last->next = NULL;
		else
			list->first = NULL;

		list->length--;

		data = node->data;
		Node_free(node, NULL);
	}
	return data;
}

static inline void
_List_prepend(List* list, void* data)
{
	Node* new = Node_new();

	new->data = data;

	Node* child = list->first;
	if (child)
	{
		child->previous = new;
		new->next = child;
		list->first = new;
	}
	else
	{
		list->first = new;
		list->last = new;
	}
	list->length++;
}

void
List_prepend(List* list, void* data)
{
	_List_prepend(list, data);
}

void
List_unshift(List* list, void* data)
{
	_List_prepend(list, data);
}

void
List_userdata_set(List* list, void* userdata)
{
	list->userdata = userdata;
}

void*
List_userdata_get(List* list)
{
	return list->userdata;
}

Node*
List_at(List* list, unsigned int index)
{
	if (index >= list->length)
		return NULL;

	unsigned int i;
	Node* node = list->first;
	for (i = 0; i < index; i++)
		node = node->next;

	return node;
}

void*
List_first(List* list)
{
	Node* node = list->first;

	if (node)
		return node->data;
	else
		return NULL;
}

void*
List_last(List* list)
{
	Node* node = list->last;

	if (node)
		return node->data;
	else
		return NULL;
}

void*
List_get(List* list, unsigned int index)
{
	Node* node = List_at(list, index);

	if (node)
		return node->data;
	else
		return NULL;
}

/* remove element at index from list */
Node*
List_detach(List* list, unsigned int index)
{
	Node* node = List_at(list, index);

	if (node)
	{
		XFDBG("detach node %u", index);
		if (node->previous)
			node->previous->next = node->next;
		else
			list->first = node->next;

		if (node->next)
			node->next->previous = node->previous;
		else
			list->last = node->previous;

		list->length--;
	}

	return node;
}

void
List_delete(List* list, unsigned int index)
{
	XFDBG("remove node %u", index);
	Node* node = List_detach(list, index);

	if (node)
		Node_free(node, list->f_node_data_free);
}

int
List_remove(List* list, void* data)
{
	Node* iter = list->first;
	Node* elem;
	unsigned int index = 0;

	LIST_EACH_WITH_INDEX(iter, elem, index)
	{
		if (elem->data == data)
		{
			XFDBG("found node to remove %u", index);
			List_delete(list, index);
			return 1;
		}
	}
	return 0;
}

// add functions: filter,reduce,map actions (like ruby)
