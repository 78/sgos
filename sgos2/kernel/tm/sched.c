//thread Scheduler

#include <sgos.h>
#include <arch.h>
#include <tm.h>
#include <mm.h>
#include <kd.h>
#include <rtl.h>

//线程调度方式
//#define SCHED_FIFO

THREAD_BOX ThreadingBox = {NULL, };
static const unsigned ms = 1000/RTC_FREQUENCY;
int CanSchedule;

// 调度初始化
void TmInitializeScheduler()
{
	RtlZeroMemory( &ThreadingBox, sizeof(THREAD_BOX) );
	CanSchedule = 0;
}

// 返回当前线程
KThread* TmGetCurrentThread()
{
	return ThreadingBox.running;
}

// 由状态获取链表
static KThread** GetThreadChain( enum THREAD_STATE st )
{
	switch( st ){
	case TS_INIT:
		break;
	case TS_DEAD:
		return &ThreadingBox.dead;
	case TS_RUNNING:
		return &ThreadingBox.running;
	case TS_READY:
		return &ThreadingBox.ready;
	case TS_SLEEP:
		return &ThreadingBox.sleep;
	case TS_PAUSED:
		return &ThreadingBox.paused;
	case TS_WAIT:
		return &ThreadingBox.wait;
	case TS_IDLE:
		return &ThreadingBox.idle;
	default:
		PERROR("##Warning: unknown thread state: 0x%X", st );
	}
	return NULL;
}

// 打印调度链表
void DumpThreadChain()
{
	uint eflags;
	int i;
	ArLocalSaveIrq(eflags);
	KThread* thr;
	thr=MmGetCurrentSpace()->FirstThread;
	for( i=0; thr; thr=thr->next, i++ )
		;
	KdPrintf("(%d) ready list: ", i);
	for( i=0, thr=ThreadingBox.ready; thr; i++, thr=thr->ScheduleInformation.next )	
		KdPrintf("[%d]", thr->ThreadId );
	KdPrintf("(%d)\n", i);
	KdPrintf("sleep list: ");
	for( i=0, thr=ThreadingBox.sleep; thr; i++, thr=thr->ScheduleInformation.next )	
		KdPrintf("[%d]", thr->ThreadId );
	KdPrintf("(%d)\n", i);
	KdPrintf("wait list: ");
	for( i=0, thr=ThreadingBox.wait; thr; i++, thr=thr->ScheduleInformation.next )	
		KdPrintf("[%d]", thr->ThreadId );
	KdPrintf("(%d)\n", i);
	KdPrintf("paused list: ");
	for( i=0, thr=ThreadingBox.paused; thr; i++, thr=thr->ScheduleInformation.next )	
		KdPrintf("[%d]", thr->ThreadId );
	KdPrintf("(%d)\n", i);
	ArLocalRestoreIrq(eflags);
}

// 设置线程状态，装入不同的链表中
void TmSetThreadState( KThread* thr, enum THREAD_STATE st )
{
	uint flags;
	KThread** link;
	SCHEDULE_INFO *info;
	if( thr->ThreadState == st )
		return;
	ArLocalSaveIrq( flags );
	info = &thr->ScheduleInformation;
	link = GetThreadChain( thr->ThreadState );
	//remove from link
	if(link && *link){
		if(info->next)
			info->next->ScheduleInformation.prev = info->prev;
		if(info->prev){
			info->prev->ScheduleInformation.next = info->next;
		}else{
			*link = info->next;
		}
	} 
	info->prev = NULL;
	//put to link
	link = GetThreadChain( st );
	if(link){
#ifdef SCHED_FIFO
		if( *link ){
			KThread* t2;
			t2 = *link;
			while( t2->ScheduleInformation.next )
				t2 = t2->ScheduleInformation.next;
			t2->ScheduleInformation.next = thr;
			info->prev = t2;
		}else{
			*link = thr;
		}
		info->next = NULL;
#else
//		抢占式。。。。
		info->next = *link;
		if( *link )
			(*link)->ScheduleInformation.prev = thr;
		*link = thr;
#endif
	}
	thr->ThreadState = st;
	//ok
	ArLocalRestoreIrq( flags );
}

// 系统时间，单位：秒
extern time_t rtc_second;
//调度器获得时钟信号
void TmIncreaseTime()
{
	KThread* thr;
	SCHEDULE_INFO* info;
	uint flags;
	//禁中断
	ArLocalSaveIrq(flags);
	//处理等待一定时间的线程
	for( thr=ThreadingBox.wait; thr; thr=thr->ScheduleInformation.next ){
		thr->ScheduleInformation.clock -= ms;
		if( thr->ScheduleInformation.clock<=0 ){
			KThread* prev = thr->ScheduleInformation.prev;
			TmSetThreadState( thr, TS_READY );
			if( prev )
				thr = prev;
			else 
				thr = ThreadingBox.wait;
			if(!thr) break;  //##important
		}
	}
	//允许中断
	ArLocalRestoreIrq(flags);
	//获取当前线程
	thr = TmGetCurrentThread();
	//是否用户态线程
	if( !thr->IsKernelThread && thr->UserModeThreadInformation ){
		//用户空间的时间变化
		thr->UserModeThreadInformation->CurrentTime = rtc_second;
	}
	//获取调度信息
	info = &thr->ScheduleInformation;
	--info->clock;
	if( info->clock <= 0 && CanSchedule==0 ){	//need scheduling
		TmSchedule();
	}
}

//调度函数，取调度列表的下一个。
//23:29 2010-2-7  HG 原来存在一个问题，我把idle线程也放进了ready列表，导致
//有一个忙的线程执行时，总是不能占满CPU，仅用了50%。增加了一个idle链，
//有忙的线程的时候不会去调idle了，保证CPU被充分使用。
void TmSchedule()
{
	KThread* thr, *cur;
	uint flags;
	cur = TmGetCurrentThread();
	//禁止中断
	ArLocalSaveIrq(flags);
	thr = NULL;
	//next one
	if( cur->ThreadState == TS_READY )
		thr = cur->ScheduleInformation.next;
	if(!thr || thr->ThreadState!=TS_READY)	//one round finished...
		thr = ThreadingBox.ready;
	if( thr ){
		// We found a ready thread.
		thr->ScheduleInformation.clock = 5;
		if( thr != cur ){
			// 指定下一个线程
			ThreadingBox.next = thr;
			// KdPrintf("[%d]", thr->ThreadId );
			// 非硬件中断下允许直接切换线程
			if( !cur->IsInterrupted )
				ArSwitchThread( cur, thr );	//线程切换
		}
	}else{
		// No ready threads. Do some idle things!
		if( thr && thr->ThreadState == TS_IDLE )
			thr = cur->ScheduleInformation.next;
		if( !thr )
			thr = ThreadingBox.idle;
		if( thr == NULL )
			KeBugCheck("No idle thread found!");
		thr->ScheduleInformation.clock = 1;
		if( thr != cur ){
			// 指定下一个线程
			ThreadingBox.next = thr;
			// 非硬件中断下允许直接切换线程
			if( !cur->IsInterrupted )
				ArSwitchThread( cur, thr );	//线程切换
		}
	}
	//back to me
	ArLocalRestoreIrq(flags);
}


