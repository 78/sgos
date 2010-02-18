//semaphore

#include <sgos.h>
#include <arch.h>
#include <kd.h>
#include <tm.h>
#include <ipc.h>
#include <mm.h>


//尝试P
int IpcTryLockSemaphore( KSemaphore *sem )
{	
	uint eflags;
	//we don't want to be interrupt
	ArLocalSaveIrq( eflags );
	while( --sem->value ){
		//don't have it..
		ArLocalRestoreIrq(eflags);
		sem->value ++;
		return 0;
	}
	//have it
	ArLocalRestoreIrq(eflags);
	return 1;
}

//P
void IpcLockSemaphore( KSemaphore *sem )
{
	uint eflags;
	//we don't want to be interrupt
	ArLocalSaveIrq( eflags );
	if( --sem->value ){
		//dont have it, then sleep
		//跟队末尾
		RtlPushFrontQueue( &sem->wait_queue, TmGetCurrentThread() );
		TmSleepThread( TmGetCurrentThread(), INFINITE );
		//now, we have it!!
	}
	ArLocalRestoreIrq(eflags);
}

//初始化
void IpcInitializeSemaphore( KSemaphore *sem )
{
	IpcInitializeSemaphoreValue( sem, 1 );
}

//初始化
void IpcInitializeSemaphoreValue( KSemaphore *sem, int v )
{
	RtlCreateQueue( &sem->wait_queue, 0, NULL, "sema_queue", 0 );
	sem->value = v;
}

//V
void IpcUnlockSemaphore( KSemaphore *sem )
{
	uint eflags;
	KThread* thr = NULL;
	ArLocalSaveIrq( eflags );
	thr = RtlPopBackQueue( &sem->wait_queue );
	ArLocalRestoreIrq( eflags );
	sem->value ++;
	if( thr ){
		TmWakeupThread( thr );
	}
}

//释放一个sema
void IpcDestroySemaphore( KSemaphore *sem )
{
	KThread* thr;
	uint eflags;
	ArLocalSaveIrq( eflags );
	while( (thr=RtlPopBackQueue(&sem->wait_queue)) ){
		TmWakeupThread( thr );
	}
	sem->value = 0;
	ArLocalRestoreIrq( eflags );
}

//删除一个链表中的项
void IpcRemoveSleepingThread( KSemaphore* sem, KThread* thr )
{
	uint eflags;
	KQueueNode *nod;
	ArLocalSaveIrq(eflags);
	thr = RtlQuickSearchQueue( &sem->wait_queue, thr, &nod );
	if( thr )
		RtlRemoveQueueElement( &sem->wait_queue, nod );
	else
		PERROR("##thread not found.");
	ArLocalRestoreIrq(eflags);
}
