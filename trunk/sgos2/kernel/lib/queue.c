/*
 *  queue.c
 *
 *  A Loop Queue
 *
 *  Copyright (C) 2008  Huang Guan
 *
 *  2008-7-12 Created.
 *  2009-8-20 Ported for SGOS2
 *
 *  Description: 循环队列，队列满时，将旧数据剔除 
 *
 */
 
#include <sgos.h>
#include <string.h>
#include <stdlib.h>
#include <thread.h>
#include <semaphore.h>
#include <mm.h>
#include <queue.h>
#include <debug.h>

//创建循环队列
int queue_create( queue_t* q, int size, queue_delete_func del, char* name )
{
	q->size = size;
	q->head = q->tail = 0;
	q->del_func = del;
	strncpy( q->name, name, QUEUE_NAME_LEN-1 );
	sema_init( &q->semaphore );
	q->items = kmalloc( q->size*sizeof(void*) );
	if( q->items == NULL )
		return -ERR_NOMEM;
	return 0;
}

//加到尾
int queue_push_to_tail( queue_t* q, void* data )
{
	sema_down( &q->semaphore );
	if( (q->tail+1)%q->size == q->head ){
		PERROR("queue %s is full. size:%d", q->name, q->size);
		if( q->del_func )
			q->del_func( q->items[q->head] );
		q->head = (q->head+1)%q->size;
		
	}
	q->items[q->tail] = data;
	q->tail = (q->tail+1)%q->size;
	sema_up( &q->semaphore );
	return 0;
}

//加到头
int queue_push_to_head( queue_t* q, void* data )
{
	sema_down( &q->semaphore );
	if( (q->size+q->head-1)%q->size == q->tail ){
		q->tail = (q->size+q->tail-1)%q->size;
		ASSERT( q->tail >= 0 );
		if( q->del_func )
			q->del_func( q->items[q->tail] );
		
	}
	q->head = (q->size+q->head-1)%q->size;
	ASSERT( q->head >= 0 );
	q->items[q->head] = data;
	sema_up( &q->semaphore );
	return 0;
}

void* queue_pop_from_head( queue_t* q )
{
	void* p = NULL;
	sema_down( &q->semaphore );
	if( q->tail != q->head ){
		p = q->items[q->head];
		q->head = (q->head+1)%q->size;
		ASSERT( q->head < q->size );
	}
	sema_up( &q->semaphore );
	return p;
}

void* queue_pop_from_tail( queue_t* q )
{
	void* p = NULL;
	sema_down( &q->semaphore );
	if( q->tail != q->head ){
		q->tail = (q->size+q->tail-1)%q->size;
		ASSERT( q->tail >= 0 );
		p = q->items[q->tail];
	}
	sema_up( &q->semaphore );
	return p;
}

//如果用链表，会快很多的。
//可以做很多优化
void queue_remove( queue_t* q, void* data )
{
	int i;
	sema_down( &q->semaphore );
	for( i=q->head; i!=q->tail; i=(i+1)%q->size )
	{
		if( q->items[i] == data ){
			q->tail = (q->size+q->tail-1)%q->size;
			ASSERT( q->tail >= 0 );
			//从i开始，数据往前移动
			for( ; i!=q->tail; i=(i+1)%q->size )
				q->items[i] = q->items[(i+1)%q->size];
			break;
		}
	}
	sema_up( &q->semaphore );
}

void* queue_search( queue_t* q, void* v, queue_search_func search )
{
	int i;
	sema_down( &q->semaphore );
	for( i=q->head; i!=q->tail; i=(i+1)%q->size )
	{
		if( search( q->items[i], v ) )
			break;
	}
	sema_up( &q->semaphore );
	if( i != q->tail )
		return q->items[i];
	return NULL;
}

void queue_cleanup( queue_t* q )
{
	int i;
	sema_down( &q->semaphore );
	if( q->del_func )
		for( i=q->head; i!=q->tail; i=(i+1)%q->size )
			q->del_func( q->items[i] );
	kfree( q->items );
	sema_destroy( &q->semaphore );
}

int queue_is_empty( queue_t* q )
{
	return( q->head == q->tail );
}

