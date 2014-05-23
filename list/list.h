#ifndef _CX_LIST_H
#define _CX_LIST_H

#include <stdio.h>

#include "base/base.h"

typedef struct cx_node_t Node;
typedef struct cx_list_t List;

typedef int F_NodeMatch (Node* node, void* key);
typedef void F_NodeDataFree (void* data);
typedef void F_NodeIterator (int index, Node* node);

struct cx_node_t
{
	Node* next;
	Node* previous;
	void* data;
};

struct cx_list_t
{
	Node* first;
	Node* last;
	unsigned long length;
	F_NodeDataFree* f_node_data_free;
	void* userdata;
};

#define LIST_EACH(head, node) \
	EACH(head, node, next)

Node*
Node_new(void);

void
Node_free(Node* node, F_NodeDataFree* f_free_node);

List*
List_new(void);

void
List_free(List* list);

/* @return the index of the appended element */
unsigned long
List_append(List* list, void* data);

/* @return the index of the pushed element */
unsigned long
List_push(List* list, void* data);

Node*
List_match(List* list, void* key, F_NodeMatch* f_node_match);

void*
List_match_node(List* list, void* key, F_NodeMatch* f_node_match);

void
List_each(List* list, F_NodeIterator* f_node_iterator);

void*
List_shift(List* list);

void*
List_pop(List* list);

void
List_prepend(List* list, void* data);

void
List_unshift(List* list, void* data);

void
List_userdata_set(List* list, void* userdata);

void*
List_userdata_get(List* list);

void*
List_get(List* list, unsigned int index);

Node*
List_at(List* list, unsigned int index);

/* remove element at index from list */
Node*
List_detach(List* list, unsigned int index);

/* delete element at index */
void
List_delete(List* list, unsigned int index);


#endif
