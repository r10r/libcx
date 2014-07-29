#ifndef _CX_LIST_CONCURRENT_H
#define _CX_LIST_CONCURRENT_H

#include <pthread.h>
#include <stdio.h>

#include <libcx/base/base.h>

#include "list.h"

typedef struct cx_list_concurrent_t
{
	List list;
	pthread_rwlock_t rwlock; /* make this a pointer ? */
} ConcurrentList;

#define __RWLOCK_LOCK_READ(list) \
	{ \
		XFDBG("lock read %p", list); \
		cx_assert(pthread_rwlock_rdlock(&list->rwlock) == 0);

#define __RWLOCK_LOCK_WRITE(list) \
	{ \
		XFDBG("lock write %p", list); \
		cx_assert(pthread_rwlock_wrlock(&list->rwlock) == 0);

#define __RWLOCK_UNLOCK(list) \
	XFDBG("unlock %p", list); \
	cx_assert(pthread_rwlock_unlock(&list->rwlock) == 0); }


#define ConcurrentList_init(list) \
	List_init((List*)list); \
	pthread_rwlock_init(&list->rwlock, NULL)

#define ConcurrentList_free(list) \
	pthread_rwlock_destroy(&list->rwlock)

#define ConcurrentList_append(list, data) \
	__RWLOCK_LOCK_WRITE(list) \
	List_append((List*)list, data); \
	__RWLOCK_UNLOCK(list)

#define ConcurrentList_push(list, data) \
	ConcurrentList_append(list, data)

#define ConcurrentList_shift(list, ptr) \
	__RWLOCK_LOCK_WRITE(list) \
	ptr = List_shift((List*)list); \
	__RWLOCK_UNLOCK(list)

#define ConcurrentList_pop(list, ptr) \
	__RWLOCK_LOCK_WRITE(list) \
	ptr = List_pop((List*)list); \
	__RWLOCK_UNLOCK(list)

#define ConcurrentList_unshift(list, data) \
	__RWLOCK_LOCK_WRITE(list) \
	List_unshift((List*)list, data); \
	__RWLOCK_UNLOCK(list)

#define ConcurrentList_prepend(list, data) \
	ConcurrentList_unshift(list, data)

#define ConcurrentList_detach(list, index) \
	__RWLOCK_LOCK_WRITE(list) \
	List_detach((List*)list, index); \
	__RWLOCK_UNLOCK(list)

#define ConcurrentList_delete(list, index) \
	__RWLOCK_LOCK_WRITE(list) \
	List_delete((List*)list, index); \
	__RWLOCK_UNLOCK(list)

#define ConcurrentList_remove(list, data) \
	__RWLOCK_LOCK_WRITE(list) \
	List_remove((List*)list, data); \
	__RWLOCK_UNLOCK(list)

/* read-only operations */

#define ConcurrentList_match(list, key, f_node_match, ptr) \
	__RWLOCK_LOCK_READ(list) \
	ptr = List_match((List*)list, key f_node_match); \
	__RWLOCK_UNLOCK(list)

#define ConcurrentList_match_node(list, key, f_node_match, ptr) \
	__RWLOCK_LOCK_READ(list) \
	ptr = List_match_node((List*)list, key f_node_match); \
	__RWLOCK_UNLOCK(list)

#define ConcurrentList_each(list, f_node_iterator, userdata) \
	__RWLOCK_LOCK_READ(list) \
	List_each((List*)list, f_node_iterator, userdata); \
	__RWLOCK_UNLOCK(list)

#define ConcurrentList_get(list, index, ptr) \
	__RWLOCK_LOCK_READ(list) \
	ptr = List_get((List*)list, index); \
	__RWLOCK_UNLOCK(list)

#define ConcurrentList_at(list, index, ptr) \
	__RWLOCK_LOCK_READ(list) \
	ptr = List_at((List*)list, index); \
	__RWLOCK_UNLOCK(list)

#define ConcurrentList_first(list, index, ptr) \
	__RWLOCK_LOCK_READ(list) \
	ptr = List_first((List*)list); \
	__RWLOCK_UNLOCK(list)

#endif
