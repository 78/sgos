#ifndef _IPC_H_
#define _IPC_H_

#include <sgos.h>
#include <rtl.h>

#define	down(a)	IpcLockSemaphore(a)
#define up(a)	IpcUnlockSemaphore(a)

struct KThread;

typedef struct KSemaphore{
	KQueue			wait_queue;
	uint			value;
}KSemaphore;

void IpcRemoveSleepingThread( KSemaphore* mut, struct KThread* thr );
int IpcTryLockSemaphore( KSemaphore *mut );
void IpcLockSemaphore( KSemaphore *mut );
void IpcInitializeSemaphore( KSemaphore *mut );
void IpcInitializeSemaphoreValue( KSemaphore *mut, int value );
void IpcUnlockSemaphore( KSemaphore *mut );
void IpcDestroySemaphore( KSemaphore *mut );


//Give it a proper value!!
#define MAX_MESSAGES_IN_QUEUE	100

//内核消息描述符
//include the message structure in user mode
typedef struct KMessage{
	Message			UserMessage;
	struct KThread *	Destination;
	struct KThread *	Source;
}KMessage;

//发送消息
//发送后返回
int	IpcSend( 
	Message*	usermsg, 	//消息正文
	uint		flag		//发送参数
);
//Quick way of IpcSend
int IpcQuickSend( uint tid, uint cmd, uint arg1, uint arg2 );

//发送后，等待对方处理完毕，超时返回负值
int	IpcCall( 
	Message*	usermsg, 	//消息正文
	uint		flag,		//发送参数
	int		timeout		//超时值
);

//接收消息
//timeout为0，表示不等待，立刻返回。
//timeout为-1，表示一直等到。
//timeout非0和-1，则等待一定的毫秒。
int	IpcReceive(
	Message*	usermsg,	//消息正文缓冲区
	uint		flag,		//接收参数
	int		timeout		//超时值
);

//回应IpcCall
int	IpcReply(
	Message*	usermsg,	//
	uint		flag		//
);

// 线程消息初始化
void IpcInitializeThreadMessageQueue( struct KThread* thr );
// 释放消息队列占用的空间
void IpcDestroyThreadMessageQueue( struct KThread* thr );

#endif
