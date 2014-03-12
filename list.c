#include "list.h"
/* NOTICE: list modification/access is not thread safe */

Node *
Node_new()
{
	Node * node = malloc(sizeof(Node));

	node->next = NULL;
	node->previous = NULL;
	return node;
}

void
Node_free(Node *node, F_NodeDataFree *f_node_data_free)
{
	if (f_node_data_free)
		f_node_data_free(node->data);
	free(node);
}

List *
List_new()
{
	List *list = malloc(sizeof(List));

	list->first = NULL;
	list->last = NULL;
	list->length = 0;
	list->f_node_data_free = NULL;
	list->userdata = NULL;
#ifndef _LIST_DISABLE_LOCKING
	list->f_lock = NULL;
	list->f_unlock = NULL;
#endif
	return list;
}

void
List_free(List *list)
{
	_LIST_LOCK_WRITE(list);
	if (list)
	{
		// FIXME use iterator to free nodes instead ?
		Node *next = list->first;

		while (next)
		{
			Node *cur = next;
			next = cur->next;
			Node_free(cur, list->f_node_data_free);
		}
		free(list);
	}
	_LIST_UNLOCK_WRITE(list);
}

static inline unsigned long
_List_push(List *list, void *data)
{
	_LIST_LOCK_WRITE(list);
	Node *new = Node_new();
	new->data = data;

	Node *parent = list->last;
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
	_LIST_UNLOCK_WRITE(list);
	return length;
}

unsigned long
List_append(List *list, void *data)
{
	return _List_push(list, data);
}

unsigned long
List_push(List *list, void *data)
{
	return _List_push(list, data);
}

Node *
List_match(List *list, void *key, F_NodeMatch *f_node_match)
{
	if (list->length == 0)
		return NULL;
	_LIST_LOCK_READ(list);
	Node *node = list->first;

	while (node)
	{
		if (f_node_match(node, key) == 0)
			break;
		node = node->next;
	}
	_LIST_UNLOCK_READ(list);
	return node;
}

void
List_each(List *list, F_NodeIterator *f_node_iterator)
{
	if (list->length == 0)
		return;
	_LIST_LOCK_READ(list);
	Node *node = list->first;

	int index = 0;
	while (node)
	{
		f_node_iterator(index, node);
		node = node->next;
		index++;
	}
	_LIST_UNLOCK_READ(list);
}

void *
List_shift(List *list)
{
	if (list->length == 0)
		return NULL;
	_LIST_LOCK_WRITE(list);
	Node *node = list->first;
	void *data = NULL;

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
	_LIST_UNLOCK_WRITE(list);
	return data;
}

void *
List_pop(List *list)
{
	if (list->length == 0)
		return NULL;
	_LIST_LOCK_WRITE(list);
	Node *node = list->last;
	void *data = NULL;
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
	_LIST_UNLOCK_WRITE(list);
	return data;
}

static inline void
_List_prepend(List *list, void *data)
{
	_LIST_LOCK_WRITE(list);
	Node *new = Node_new();
	new->data = data;

	Node *child = list->first;
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
	_LIST_UNLOCK_WRITE(list);
}

void
List_prepend(List *list, void *data)
{
	_List_prepend(list, data);
}

void
List_unshift(List *list, void *data)
{
	_List_prepend(list, data);
}

void
List_userdata_set(List *list, void *userdata)
{
	list->userdata = userdata;
}

void *
List_userdata_get(List *list)
{
	return list->userdata;
}
