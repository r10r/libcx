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
Node_free_data(void* data)
{
	cx_free(data);
}

List*
List_new()
{
	List* list = cx_alloc(sizeof(List));

	list->f_node_data_free = Node_free_data;
	return list;
}

void
List_free(List* list)
{
	if (list)
	{
		// FIXME use iterator to free nodes instead ?
		Node* next = list->first;
		while (next)
		{
			Node* cur = next;
			next = cur->next;
			Node_free(cur, list->f_node_data_free);
		}
		cx_free(list);
	}
}

static inline unsigned long
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
	unsigned long length = list->length++;
	return length;
}

unsigned long
List_append(List* list, void* data)
{
	return _List_push(list, data);
}

unsigned long
List_push(List* list, void* data)
{
	return _List_push(list, data);
}

Node*
List_match(List* list, void* key, F_NodeMatch* f_node_match)
{
	if (list->length == 0)
		return NULL;

	Node* node = list->first;
	while (node)
	{
		if (f_node_match(node, key) == 0)
			return node;
		else
			node = node->next;
	}
	return NULL;
}

void*
List_match_node(List* list, void* key, F_NodeMatch* f_node_match)
{
	Node* node = List_match(list, key, f_node_match);

	if (node)
		return node->data;
	else
		return NULL;
}

void
List_each(List* list, F_NodeIterator* f_node_iterator)
{
	if (list->length == 0)
		return;
	Node* node = list->first;

	int index = 0;
	while (node)
	{
		f_node_iterator(index, node);
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
	Node* node = List_detach(list, index);

	if (node)
		Node_free(node, list->f_node_data_free);
}

// add functions: filter,reduce,map actions (like ruby)
