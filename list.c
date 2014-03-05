/*
 * Generic linked list implementation.
 */
#include "list.h"

static int nodes_created = 0;

Node *
Node_new()
{
	Node * node = malloc(sizeof(Node));
	node->next = NULL;
	nodes_created++;
	return node;
}

List *
List_new()
{
	List *list = malloc(sizeof(List));
	list->first = NULL;
	list->last = NULL;
	list->length = 0;
	list->free_node_cb = NULL;
	list->hash_node_cb = NULL;
	return list;
}

void
Node_destroy(Node *node, void(*free_node_cb)(Node *node))
{
	free_node_cb(node);
	free(node);
}

void
List_append(List *list, void *data)
{
	Node *node = Node_new();
	node->data.value = data;
	if (list->hash_node_cb)
		list->hash_node_cb(node);

	Node *parent = list->last;
	if (parent)
	{
		parent->next = node;
		list->last = node;
	}
	else
	{
		list->first = node;
		list->last = node;
	}

	list->length++;
}

void
List_destroy(List *list)
{
	Node *next = list->first;
	while(next)
	{
		Node *cur = next;
		next = cur->next;
		Node_destroy(cur, list->free_node_cb);
	}
	free(list);
}

Node *
List_find(List *list, void *key, int(*comp)(Node *node, void *key))
{
	Node *node = list->first;
	while(node)
	{
		if (comp(node, key) == 0) return node;
		node = node->next;
	}
	return NULL;
}

void
List_each(List *list, void(*iterator)(Node *node))
{
	Node *node = list->first;
	while(node)
	{
		iterator(node);
		node = node->next;
	}
}
