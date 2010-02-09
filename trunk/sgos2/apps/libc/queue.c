/*
 *  queue.c
 *
 *  A Loop Queue
 *
 *  Copyright (C) 2008  Huang Guan
 *
 *  2008-7-12 Created.
 *  2009-8-20 Ported for SGOS2
 *  2009-8-30 Use link table instead of array
 *
 *  Description: 循环队列，队列满时，将旧数据剔除 
 *  front---1---2---3----4---5---6---back
 *  这里插入                        这里取出
 */
 
#include <sgos.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <queue.h>

//创建循环队列
int queue_create( queue_t* q, int size, queue_delete_func del, const char* name, int use_sem )
{
	if( size==0 )
		size = 0x70000000;
	q->max_num = size;
	q->front = q->back = NULL;
	q->del_func = del;
	q->cur_num = 0;
	q->use_sem = use_sem;
	strncpy( q->name, name, QUEUE_NAME_LEN-1 );
	/* sem not supported yet 
	if( q->use_sem ){
		q->semaphore = MmAllocateKernelMemory(sizeof(sema_t));
		if(!q->semaphore)
			return -ERR_NOMEM;
		sema_init( q->semaphore );
	}*/
	return 0;
}

//加到尾
int queue_push_back( queue_t* q, void* data )
{
	qnode_t* nod;
	if( q->cur_num>=q->max_num )
		return -ERR_NOMEM;
	nod = malloc(sizeof(qnode_t));
	if(!nod)
		return -ERR_NOMEM;
	nod->v = data;
	q->cur_num++;
	//进入临界区
//	if( q->use_sem )
//		sema_down( q->semaphore );
	//链表处理，加入到back后
	if( q->back )
		q->back->pre = nod;
	else if(!q->front)
		q->front = nod;
	nod->pre = NULL;
	nod->next = q->back;
	q->back = nod;
	//离开临界区
//	if( q->use_sem )
//		sema_up( q->semaphore );
	return 0;
}

//加到头
int queue_push_front( queue_t* q, void* data )
{
	qnode_t* nod;
	if( q->cur_num>=q->max_num ){
		printf("##QUEUE %s is full.", q->name );
		return -ERR_NOMEM;
	}
	nod = malloc(sizeof(qnode_t));
	if(!nod)
		return -ERR_NOMEM;
	nod->v = data;
	q->cur_num++;
	//进入临界区
//	if( q->use_sem )
//		sema_down( q->semaphore );
	//处理链表
	if( q->front )
		q->front->next = nod;
	else if( !q->back )	
		q->back = nod;
	nod->pre = q->front;
	nod->next = NULL;
	q->front = nod;
	//离开临界区
//	if( q->use_sem )
//		sema_up( q->semaphore );
	return 0;
}

void* queue_pop_front( queue_t* q )
{
	qnode_t* tmp = NULL;
	void* p = NULL;
	if( q->cur_num == 0 )
		return NULL;
	//进入临界区
//	if( q->use_sem )
//		sema_down( q->semaphore );
	if( q->front ){
		p = q->front->v;
		if( q->front->pre )
			q->front->pre->next = NULL;
		tmp = q->front;
		q->front = q->front->pre;
		if( !q->front )
			q->back = NULL;
	}
	//离开临界区
//	if( q->use_sem )
//		sema_up( q->semaphore );
	q->cur_num --;
	if(tmp)
		free( tmp );
	return p;
}

void* queue_pop_back( queue_t* q )
{
	qnode_t* tmp = NULL;
	void* p = NULL;
	//进入临界区
//	if( q->use_sem )
//		sema_down( q->semaphore );
	if( q->back ){
		p = q->back->v;
		if( q->back->next )
			q->back->next->pre = NULL;
		tmp = q->back;
		q->back = q->back->next;
		if( !q->back )
			q->front = NULL;
	}
	//离开临界区
//	if( q->use_sem )
//		sema_up( q->semaphore );
	q->cur_num --;
	if(tmp)
		free( tmp );
	return p;
}

//链表，会比数组快很多的。
void queue_remove( queue_t* q, qnode_t* nod )
{
	//进入临界区
//	if( q->use_sem )
//		sema_down( q->semaphore );
	if( nod->pre )
		nod->pre->next = nod->next;
	else
		q->back = NULL;
	if( nod->next )
		nod->next->pre = nod->pre;
	else
		q->front = NULL;
	//离开临界区
//	if( q->use_sem )
//		sema_up( q->semaphore );
	free( nod );
}

void* queue_search( queue_t* q, void* v, queue_search_func search, qnode_t** ret_nod )
{
	qnode_t * nod;
	//进入临界区
//	if( q->use_sem )
//		sema_down( q->semaphore );
	for( nod=q->front; nod; nod=nod->pre )
	{
		if( search( nod->v, v ) )
			break;
	}
	//离开临界区
//	if( q->use_sem )
//		sema_up( q->semaphore );
	*ret_nod = nod;
	if( nod )
		return nod->v;
	else
		return NULL;
}

void* queue_quick_search( queue_t* q, void* v, qnode_t** ret_nod )
{
	qnode_t * nod;
	//进入临界区
//	if( q->use_sem )
//		sema_down( q->semaphore );
	for( nod=q->front; nod; nod=nod->pre )
	{
		if( nod->v == v )
			break;
	}
	//离开临界区
//	if( q->use_sem )
//		sema_up( q->semaphore );
	*ret_nod = nod;
	if( nod )
		return nod->v;
	else
		return NULL;
}

void queue_cleanup( queue_t* q )
{
	q->cur_num = 1;
	q->max_num = 0;
	if(q->del_func)
		while(!queue_is_empty(q))
			q->del_func(queue_pop_back(q));
	else
		while(!queue_is_empty(q))
			queue_pop_back(q);
//	if(q->use_sem){
//		sema_destroy(q->semaphore);
//		free(q->semaphore);
//	}
}

int queue_is_empty( queue_t* q )
{
	return( q->front == NULL );
}

