#ifndef _CX_LIST_H
#define _CX_LIST_H

#include <stdio.h>

#include <libcx/base/base.h>

typedef struct cx_node_t Node;
typedef struct cx_list_t List;

typedef long F_NodeDataCompare (Node* node, const void* key);
typedef void F_NodeDataFree (void* data);

/* @return
 *  1 to continue iteration
 *  0 to stop iteration
 */
typedef int F_NodeIterator (int index, Node* node, void* userdata);

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
	long length;
	F_NodeDataFree* f_node_data_free;
	F_NodeDataCompare* f_node_data_compare;
	void* userdata;
};

#define LIST_EACH(iter, elem) \
	EACH(iter, elem, next)

#define LIST_EACH_WITH_INDEX(iter, elem, index) \
	EACH_WITH_INDEX(iter, elem, next, index)

Node*
Node_new(void);

void
Node_free(Node* node, F_NodeDataFree* f_free_node);

void
List_init(List* list);

List*
List_new(void);

void
List_free_members(List* list);

void
List_free(List* list);

/* @return the index of the appended element */
long
List_append(List* list, void* data);

/* @return the index of the pushed element */
long
List_push(List* list, void* data);

Node*
List_match(List* list, const void* key);

void*
List_match_node(List* list, const void* key);

void
List_each(List* list, F_NodeIterator* f_node_iterator, void* userdata);

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

void*
List_last(List* list);

void*
List_first(List* list);

/* @return 1 when node was found and deleted, 0 else */
int
List_remove(List* list, void* data);

#endif
