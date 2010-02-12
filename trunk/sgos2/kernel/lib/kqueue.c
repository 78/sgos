/*
 *  queue.c
 *
 *  A Loop Queue
 *
 *  Copyright (C) 2008  Xiaoxia
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
#include <stdlib.h>
#include <tm.h>
#include <ipc.h>
#include <mm.h>
#include <rtl.h>
#include <kd.h>

//创建循环队列
int RtlCreateQueue( KQueue* q, int size, KQueueEraser del, const char* name, int use_sem )
{
	if( size==0 )
		size = 0x70000000;
	q->max_num = size;
	q->front = q->back = NULL;
	q->del_func = del;
	q->cur_num = 0;
	q->use_sem = use_sem;
//	strncpy( q->name, name, QUEUE_NAME_LEN-1 );
	if( q->use_sem ){
		q->semaphore = MmAllocateKernelMemory(sizeof(KSemaphore));
		if(!q->semaphore)
			return -ERR_NOMEM;
		IpcInitializeSemaphore( q->semaphore );
	}
	return 0;
}

//加到尾
int RtlPushBackQueue( KQueue* q, void* data )
{
	KQueueNode* nod;
	if( q->cur_num>=q->max_num )
		return -ERR_NOMEM;
	nod = MmAllocateKernelMemory(sizeof(KQueueNode));
	if(!nod)
		return -ERR_NOMEM;
	nod->v = data;
	q->cur_num++;
	//进入临界区
	if( q->use_sem )
		IpcLockSemaphore( q->semaphore );
	//链表处理，加入到back后
	if( q->back )
		q->back->prev = nod;
	else if(!q->front)
		q->front = nod;
	nod->prev = NULL;
	nod->next = q->back;
	q->back = nod;
	//离开临界区
	if( q->use_sem )
		IpcUnlockSemaphore( q->semaphore );
	return 0;
}

//加到头
int RtlPushFrontQueue( KQueue* q, void* data )
{
	KQueueNode* nod;
	if( q->cur_num>=q->max_num ){
		PERROR("##QUEUE %s is full.", q->name );
		return -ERR_NOMEM;
	}
	nod = MmAllocateKernelMemory(sizeof(KQueueNode));
	if(!nod)
		return -ERR_NOMEM;
	nod->v = data;
	q->cur_num++;
	//进入临界区
	if( q->use_sem )
		IpcLockSemaphore( q->semaphore );
	//处理链表
	if( q->front )
		q->front->next = nod;
	else if( !q->back )	
		q->back = nod;
	nod->prev = q->front;
	nod->next = NULL;
	q->front = nod;
	//离开临界区
	if( q->use_sem )
		IpcUnlockSemaphore( q->semaphore );
	return 0;
}

void* RtlPopFrontQueue( KQueue* q )
{
	KQueueNode* tmp = NULL;
	void* p = NULL;
	if( q->cur_num == 0 )
		return NULL;
	//进入临界区
	if( q->use_sem )
		IpcLockSemaphore( q->semaphore );
	if( q->front ){
		p = q->front->v;
		if( q->front->prev )
			q->front->prev->next = NULL;
		tmp = q->front;
		q->front = q->front->prev;
		if( !q->front )
			q->back = NULL;
	}
	//离开临界区
	if( q->use_sem )
		IpcUnlockSemaphore( q->semaphore );
	q->cur_num --;
	if(tmp)
		MmFreeKernelMemory( tmp );
	return p;
}

void* RtlPopBackQueue( KQueue* q )
{
	KQueueNode* tmp = NULL;
	void* p = NULL;
	//进入临界区
	if( q->use_sem )
		IpcLockSemaphore( q->semaphore );
	if( q->back ){
		p = q->back->v;
		if( q->back->next )
			q->back->next->prev = NULL;
		tmp = q->back;
		q->back = q->back->next;
		if( !q->back )
			q->front = NULL;
	}
	//离开临界区
	if( q->use_sem )
		IpcUnlockSemaphore( q->semaphore );
	q->cur_num --;
	if(tmp)
		MmFreeKernelMemory( tmp );
	return p;
}

//链表，会比数组快很多的。
void RtlRemoveQueueElement( KQueue* q, KQueueNode* nod )
{
	//进入临界区
	if( q->use_sem )
		IpcLockSemaphore( q->semaphore );
	if( nod->prev )
		nod->prev->next = nod->next;
	else
		q->back = NULL;
	if( nod->next )
		nod->next->prev = nod->prev;
	else
		q->front = NULL;
	//离开临界区
	if( q->use_sem )
		IpcUnlockSemaphore( q->semaphore );
	MmFreeKernelMemory( nod );
}

void* RtlSearchQueue( KQueue* q, void* v, KQueueSearcher search, KQueueNode** ret_nod )
{
	KQueueNode * nod = NULL;
	//进入临界区
	if( q->use_sem )
		IpcLockSemaphore( q->semaphore );
	for( nod=q->front; nod; nod=nod->prev )
	{
		if( search( nod->v, v ) )
			break;
	}
	//离开临界区
	if( q->use_sem )
		IpcUnlockSemaphore( q->semaphore );
	*ret_nod = nod;
	if( nod )
		return nod->v;
	else
		return NULL;
}

void* RtlQuickSearchQueue( KQueue* q, void* v, KQueueNode** ret_nod )
{
	KQueueNode * nod = NULL;
	//进入临界区
	if( q->use_sem )
		IpcLockSemaphore( q->semaphore );
	for( nod=q->front; nod; nod=nod->prev )
	{
		if( nod->v == v )
			break;
	}
	//离开临界区
	if( q->use_sem )
		IpcUnlockSemaphore( q->semaphore );
	*ret_nod = nod;
	if( nod )
		return nod->v;
	else
		return NULL;
}

void RtlDestroyQueue( KQueue* q )
{
	q->cur_num = 1;
	q->max_num = 0;
	if(q->del_func)
		while(!RtlIsEmptyQueue(q))
			q->del_func(RtlPopBackQueue(q));
	else
		while(!RtlIsEmptyQueue(q))
			RtlPopBackQueue(q);
	if(q->use_sem){
		IpcDestroySemaphore(q->semaphore);
		MmFreeKernelMemory(q->semaphore);
	}
}

int RtlIsEmptyQueue( KQueue* q )
{
	return( q->front == NULL );
}

