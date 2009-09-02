//message management.

#include <sgos.h>
#include <mm.h>
#include <debug.h>
#include <message.h>
#include <thread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <string.h>

// 删除一个消息
static void message_delete(const void * p)
{
	KMESSAGE* kmsg = (KMESSAGE*)p;
	kfree( kmsg->content );
	kfree( kmsg );
}

// 给出source线程查找一个消息
static int message_search(const void * p, const void *q )
{
	if( ((KMESSAGE*)p)->source == q )
		return 1;
	return 0;	//search next
}

// 线程消息初始化
void message_init( struct THREAD* thr )
{
	char tmp[16];
	sprintf( tmp, "msgQue:%x", thr->tid );
	queue_create( &thr->message_queue, MAX_MESSAGES_IN_QUEUE, message_delete, tmp, 1 );
}

// 释放消息队列占用的空间
void message_destroy( struct THREAD* thr )
{
	queue_cleanup( &thr->message_queue );
}

// Send a message to another thread.
int message_send( session_t* session, void* content, size_t len, uint flag )
{
	THREAD* thr_dest;
	KMESSAGE* kmsg;
	thr_dest = (THREAD*)session->thread;
	down( &thr_dest->semaphore );
	/* 在睡眠醒来后，目的线程可能已经终止，所以要检查状态 */
	if( thr_dest->state == TS_DEAD || thr_dest->state == TS_INIT ){
		up( &thr_dest->semaphore );
		return -ERR_WRONGARG;
	}
	kmsg = kmalloc( sizeof(KMESSAGE) );
	if( !kmsg ){
		up( &thr_dest->semaphore );
		return -ERR_NOMEM;
	}
	//copy arguments
	kmsg->session = *session;
	kmsg->length = len;
	kmsg->dest = thr_dest;
	kmsg->source = current_thread();
	kmsg->flag = flag;
	kmsg->content = kmalloc( len );
	if( !kmsg->content ){
		kfree(kmsg);
		up( &thr_dest->semaphore );
		return -ERR_NOMEM;
	}
	//copy from the caller's memory space
	memcpy( kmsg->content, content, len );
	//insert message queue
	queue_push_front( &thr_dest->message_queue, kmsg );
	//finished
	up( &thr_dest->semaphore );
	//wake up the thread
	if( thr_dest->state == TS_SLEEP )
		thread_wakeup( thr_dest );
	return 0;	//success
}

// Receive a message
int message_recv( session_t* session, void* content, size_t* len, uint flag )
{
	THREAD* thr_src, * thr_cur;
	KMESSAGE* kmsg;
	qnode_t* nod;
	int ret;
	thr_src = (THREAD*)session->thread;
	thr_cur = current_thread();
	thr_src = NULL;
_recv_search:
	if( thr_src )	//specify the thread
		kmsg = queue_search( &thr_cur->message_queue, thr_src,
			message_search, &nod );
	else	//receive any message
		kmsg = queue_pop_back( &thr_cur->message_queue );
	if( !kmsg ){
		if( flag&MSG_PENDING ){
			thread_sleep();
			goto _recv_search;
		}
		//要求立即返回，但又没有取得消息
		return -ERR_NONE;
	}
	//check user space
	if( kmsg->length > *len ){
		*len = kmsg->length;
		//恢复取出来的消息
		if(!thr_src )
			queue_push_back( &thr_cur->message_queue, kmsg );
		return -ERR_NOMEM;
	}
	//set return value
	ret = kmsg->length;
	//copy to user space
	memcpy( content, kmsg->content, ret );
	//check flag
	if( thr_src ){
		if( !(flag&MSG_KEEP ) ){
			queue_remove( &thr_cur->message_queue, nod );
			message_delete( kmsg );
		}
	}else{
		if( flag&MSG_KEEP )
			queue_push_back( &thr_cur->message_queue, kmsg );
		else
			message_delete( kmsg );
	}
	//设置来源
	session->thread = (uint)kmsg->source;
	session->process = (uint)((THREAD*)kmsg->source)->process;
	return ret;
}

