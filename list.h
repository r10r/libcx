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

/* __attribute__((__packed__)) */
typedef struct  hashed_data_t
{
	void *value;
	unsigned long hash;
} Data;

typedef struct node_t
{
	Data data;
	struct node_t *next;
} Node;

typedef struct list_t
{
	Node *first;
	Node *last;
	long length;
	void(*free_node_cb)(Node *node);
	void(*hash_node_cb)(Node *node);
} List;

Node *
Node_new();

List *
List_new();

void
List_destroy(List *list);

void
Node_destroy(Node *node, void(*free_node_cb)(Node *node));

void
List_append(List *list, void *data);

Node *
List_find(List *list, void *key, int(*comp)(Node *node, void *key));

void
List_each(List *list, void(*iterator)(Node *node));

#endif
