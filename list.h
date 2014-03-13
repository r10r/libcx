#ifndef _LIST_H
#define _LIST_H

/*
 * http://stackoverflow.com/questions/487258/plain-english-explanation-of-big-o/487278#487278
 * http://stackoverflow.com/questions/393556/when-to-use-a-linked-list-over-an-array-array-list
 * http://pseudomuto.com/development/2013/05/02/implementing-a-generic-linked-list-in-c.html
 * https://www.cs.auckland.ac.nz/software/AlgAnim/ds_ToC.html
 */

#include <stdio.h>
#include <stdlib.h> /* malloc */
#include <libcx-base/debug.h>

typedef struct node_t Node;
typedef struct list_t List;

typedef void F_ListLock (List *list, int rw);

typedef int F_NodeMatch (Node *node, void *key);
typedef void F_NodeDataFree (void *data);
typedef void F_NodeIterator (int index, Node *node);

/* multiple readers, one writer locking */
#ifdef _LIST_DISABLE_LOCKING
/* locking support is disabled */
#define _LIST_LOCK_READ(list)
#define _LIST_LOCK_WRITE(list)
#define _LIST_UNLOCK_READ(list)
#define _LIST_UNLOCK_WRITE(list)
#else
/* locking support is enabled */
#define _LIST_LOCK_READ(list) \
	if (list->f_lock) { XDBG("lock READ"); list->f_lock(list, 0); }
#define _LIST_LOCK_WRITE(list) \
	if (list->f_lock) { XDBG("lock WRITE"); list->f_lock(list, 1); }
#define _LIST_UNLOCK_READ(list) \
	if (list->f_unlock) { XDBG("unlock READ"); list->f_unlock(list, 0); }
#define _LIST_UNLOCK_WRITE(list) \
	if (list->f_unlock) { XDBG("unlock WRITE"); list->f_unlock(list, 1); }
#endif

struct node_t
{
	Node *next;
	Node *previous;
	void *data;
};

struct list_t
{
	Node *first;
	Node *last;
	unsigned long length;
	F_NodeDataFree *f_node_data_free;
	void *userdata;
#ifndef _LIST_DISABLE_LOCKING
	F_ListLock *f_lock;
	F_ListLock *f_unlock;
#endif
};

Node *
Node_new(void);

void
Node_free(Node *node, F_NodeDataFree *f_free_node);

List *
List_new(void);

void
List_free(List *list);

/* @return the index of the appended element */
unsigned long
List_append(List *list, void *data);

/* @return the index of the pushed element */
unsigned long
List_push(List *list, void *data);

Node *
List_match(List *list, void *key, F_NodeMatch *f_node_match);

void
List_each(List *list, F_NodeIterator *f_node_iterator);

void *
List_shift(List *list);

void *
List_pop(List *list);

///* @return data from element removed from given index */
//void *
//List_remove(List *list, int index);
//
///* @return data from node at given index */
//void *
//List_get(List *list, int index);

void
List_prepend(List *list, void *data);

void
List_unshift(List *list, void *data);

void
List_userdata_set(List *list, void *userdata);

void *
List_userdata_get(List *list);

void *
List_get(List *list, unsigned int index);

#endif
