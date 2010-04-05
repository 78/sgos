//SGOS2 thread

#include <sgos.h>
#include <arch.h>
#include <tm.h>
#include <mm.h>
#include <kd.h>
#include <rtl.h>

// useless at the present
static unsigned short threadId = 1;

// thread management.
void TmInitializeThreadManagement()
{
	threadId = 1;
}

// just for fun :-)
static uint GenerateThreadId(KSpace* space)
{
	uint tid = threadId;
	if( (++threadId)%4 == 0 )
		++threadId;
	return (space->SpaceId<<16) | tid;
}

//初始化用户态线程信息块
/*
static void InitializeThreadInformation( KThread* thr )
{
	thr->UserModeThreadInformation = MmAllocateUserMemory(thr->Space, 
		PAGE_ALIGN(sizeof(ThreadInformation)), 0, 0);
	if( thr->UserModeThreadInformation ){
		ThreadInformation* ti = thr->UserModeThreadInformation;
		//清0操作，以免用户获得错误信息
		RtlZeroMemory( ti, PAGE_ALIGN(sizeof(ThreadInformation)) );
		//线程堆栈地址
		ti->StackBase = thr->StackBase;
		//线程堆栈大小
		ti->StackLimit = thr->StackLimit;
		//地址空间信息指针
		ti->SpaceInformation = thr->Space->UserModeSpaceInformation;
		//指向自己
		ti->Self = ti;
		//地址空间id
		ti->SpaceId = thr->Space->SpaceId;
		ti->ThreadId = thr->ThreadId;
	}
}
*/

//由线程ID获得线程结构指针
KThread* TmGetThreadById( uint tid )
{
	KSpace* space;
	KThread* thr;
	uint flags;
	//看是否当前线程tid
	thr = TmGetCurrentThread();
	if( thr->ThreadId == tid )
		return thr;
	//否则在地址空间中查找
	space = MmGetSpaceById(tid>>16);
	if( space == NULL )
		return NULL;
	//是否需要禁中断呢？？
	ArLocalSaveIrq(flags);
	for( thr=space->FirstThread; thr; thr=thr->next ){
		if( thr->ThreadId == tid ){
			ArLocalRestoreIrq(flags);
			return thr;
		}
	}
	ArLocalRestoreIrq(flags);
	KdPrintf("Thread %x not found.\n", tid );
	return NULL;
}

//创建线程，初始化相关信息
KThread* TmCreateAdvancedThread( KSpace* space, size_t entry_addr, size_t stack_limit, size_t stack_base, ThreadInformation* ti, int flag )
{
	KThread* thr;
	SCHEDULE_INFO* sched;
	uint eflags;
	//分配线程内存空间
	thr = (KThread*)MmAllocateKernelMemory( sizeof(KThread) );
	if( thr == NULL ){
		PERROR("## kernel memory used out!!");
		return NULL;
	}
	RtlZeroMemory( thr, sizeof(KThread) );
	//线程标识
	thr->Magic = THREAD_MAGIC;
	//初始化线程信号量
	IpcInitializeSemaphore( &thr->Semaphore );
	//初始化消息队列
	IpcInitializeThreadMessageQueue( thr );
	//初始化等待队列
	IpcInitializeSemaphoreValue( &thr->JoinSemaphore, 0 );
	//调度信息
	sched = &thr->ScheduleInformation;
	//线程基本信息
	thr->ThreadId = GenerateThreadId(space);
	thr->Space = space;
	thr->ThreadState = TS_INIT;
	thr->SchedulePriority = PRI_NORMAL;
	thr->UserModeThreadInformation = ti;
	thr->EntryAddress = entry_addr;
	thr->StackLimit = stack_limit;
	if( flag & BIOS_THREAD )
		thr->InBiosMode = 1;
	if( flag & KERNEL_THREAD ){//创建用户态线程？？
		thr->IsKernelThread = 1;
	}else{
		thr->IsKernelThread = 0;
		//用户态线程堆栈
		if( !thr->InBiosMode){
			thr->StackLimit = stack_limit;
			thr->StackBase = stack_base;
			if( stack_limit < PAGE_SIZE )
				thr->StackLimit = THREAD_STACK_SIZE;
			if( stack_base == 0 )
				thr->StackBase = (uint)MmAllocateUserMemory( space, thr->StackLimit, 
					PAGE_ATTR_WRITE, 0);
			if( ti == NULL )
				thr->UserModeThreadInformation = (ThreadInformation*)MmAllocateUserMemory( space, PAGE_SIZE, 
					PAGE_ATTR_WRITE, ALLOC_ZERO|ALLOC_HIGHMEM );
		}
	}
	//初始化寄存器
	ArInitializeThreadRegisters( thr, TmGetCurrentThread(), NULL, entry_addr, 
		thr->StackBase + thr->StackLimit );
	//进行链表操作
	ArLocalSaveIrq( eflags );
	//插队
	thr->next = space->FirstThread;
	if( space->FirstThread )
		space->FirstThread->prev = thr;
	space->FirstThread = thr;
	ArLocalRestoreIrq( eflags );
	//添加到thread_box
	TmSetThreadState( thr, TS_PAUSED );
	//return the pointer
	return thr;
}

//创建一个线程
KThread* TmCreateThread( KSpace* space, size_t entry_addr, uint flag )
{
	return TmCreateAdvancedThread( space, entry_addr, 0, 0, NULL, flag );
}

//结束线程
int TmTerminateThread( KThread* thr, uint code )
{
	KSpace* space;
	uint flags;
	if( thr == NULL ){
		PERROR("thr == NULL");
		return -ERR_WRONGARG;
	}
//	KdPrintf("Thread %x exited with code: %d\n", thr->ThreadId, code );
	space = thr->Space;
	//如果线程睡眠了，怎么办？
	down( &thr->Semaphore );
	if( thr == TmGetCurrentThread() )
		//禁止切换线程
		ArLocalSaveIrq(flags);
	//设置状态为死亡
	TmSetThreadState( thr, TS_DEAD );
	//线程退出码
	thr->ExitCode = code;
	//清空消息队列
	IpcDestroyThreadMessageQueue( thr );
	//向各系统服务发送线程结束消息
	//....
	//回收资源
	ArReleaseThreadResources( thr );
	//回收堆栈
	if( !thr->IsKernelThread && !thr->InBiosMode )//用户态线程？？
		MmFreeUserMemory( space, (void*)thr->StackBase );
	//Wakeup all related sleeping threads
	IpcDestroySemaphore( &thr->Semaphore );
	//唤醒所有join线程
	IpcDestroySemaphore( &thr->JoinSemaphore );
	if( thr == TmGetCurrentThread() )
		ArLocalRestoreIrq(flags);
	//线程切换
	TmSchedule();
	return 0;
}

//等待一个线程结束
void TmJoinThread( KThread* thr )
{
	KSpace* space;
	KThread* t;
	uint flags;
	space = MmGetCurrentSpace();
	ArLocalSaveIrq( flags );
	//为什么要搜索一遍呢? 有待分析...
	for( t=space->FirstThread; t; t=t->next )
		if( t==thr ){
			if( thr->ThreadState != TS_DEAD )
				IpcLockSemaphore( &thr->JoinSemaphore );
			break;
		}
	ArLocalRestoreIrq( flags );
}

//线程继续
int TmResumeThread( KThread* thr )
{
	TmSetThreadState( thr, TS_READY );
	return 0;
}

//挂起线程
int TmSuspendThread( KThread* thr )
{
	TmSetThreadState( thr, TS_PAUSED );
	if( thr == TmGetCurrentThread() )
		TmSchedule();
	return 0;
}

//睡眠线程。注意，中断情况下不可以睡眠。
int TmSleepThread(KThread* thread, uint ms)
{
	if( ms == INFINITE ){
		TmSetThreadState( thread, TS_SLEEP );
		if( thread == TmGetCurrentThread() ){
			TmSchedule();
			ASSERT( thread->ThreadState != TS_SLEEP );
		}
	}else{
		uint flags;
		//关中断，禁止线程切换
		ArLocalSaveIrq( flags );
		//如果此时被中断，则clock可能被更改了。。。
		thread->ScheduleInformation.clock = ms;
		TmSetThreadState( thread, TS_WAIT );
		ArLocalRestoreIrq( flags );
		if( thread == TmGetCurrentThread() ){
			TmSchedule();
			ASSERT( thread->ThreadState != TS_WAIT );
		}
	}
	return 0;
}

//唤醒线程
int TmWakeupThread( KThread* thr )
{
	if( thr->ThreadState == TS_SLEEP ||  thr->ThreadState == TS_WAIT ){
		TmSetThreadState( thr, TS_READY );
		return 0;
	}
	return -ERR_UNKNOWN;
}

//引发一个事件，让线程从内核态返回用户态后执行。
void TmRaiseThreadEvent( KThread* thr, uint addr )
{
	//这个有点复杂。。。
	KeBugCheck("not implemented.");
}
