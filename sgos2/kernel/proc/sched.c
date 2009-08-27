//thread schedule

#include <sgos.h>
#include <arch.h>
#include <thread.h>
#include <process.h>
#include <string.h>
#include <debug.h>

//线程调度方式
//#define SCHED_FIFO

THREAD_BOX tbox = {NULL, };
static const unsigned ms = 1000/RTC_FREQUENCY;
int sched_off;

// 调度初始化
void sched_init()
{
	memset( &tbox, 0, sizeof(THREAD_BOX) );
	sched_off = 0;
}

// 返回当前线程
THREAD* current_thread()
{
	return tbox.running;
}

// 由状态获取链表
THREAD** get_thread_link( enum THREAD_STATE st )
{
	switch( st ){
	case TS_INIT:
		break;
	case TS_DEAD:
		return &tbox.dead;
	case TS_RUNNING:
		return &tbox.running;
	case TS_READY:
		return &tbox.ready;
	case TS_SLEEP:
		return &tbox.sleep;
	case TS_PAUSED:
		return &tbox.paused;
	case TS_WAIT:
		return &tbox.wait;
	default:
		PERROR("##Warning: unknown thread state: 0x%X", st );
	}
	return NULL;
}

// 打印调度链表
void dump_link()
{
	uint eflags;
	int i;
	local_irq_save(eflags);
	THREAD* thr;
	thr=current_proc()->thread;
	for( i=0; thr; thr=thr->next, i++ )
		;
	kprintf("(%d) ready list: ", i);
	for( i=0, thr=tbox.ready; thr; i++, thr=thr->sched_info.next )	
		kprintf("[%d]", thr->tid );
	kprintf("(%d)\n", i);
	kprintf("sleep list: ");
	for( i=0, thr=tbox.sleep; thr; i++, thr=thr->sched_info.next )	
		kprintf("[%d]", thr->tid );
	kprintf("(%d)\n", i);
	kprintf("wait list: ");
	for( i=0, thr=tbox.wait; thr; i++, thr=thr->sched_info.next )	
		kprintf("[%d]", thr->tid );
	kprintf("(%d)\n", i);
	kprintf("paused list: ");
	for( i=0, thr=tbox.paused; thr; i++, thr=thr->sched_info.next )	
		kprintf("[%d]", thr->tid );
	kprintf("(%d)\n", i);
	local_irq_restore(eflags);
}

// 设置线程状态，装入不同的链表中
void sched_set_state( THREAD* thr, enum THREAD_STATE st )
{
	uint flags;
	THREAD** link;
	SCHEDULE_INFO *info;
	if( thr->state == st )
		return;
	local_irq_save( flags );
	info = &thr->sched_info;
	link = get_thread_link( thr->state );
	//remove from link
	if(link && *link){
		if(info->next)
			info->next->sched_info.pre = info->pre;
		if(info->pre){
			info->pre->sched_info.next = info->next;
		}else{
			*link = info->next;
		}
	} 
	info->pre = NULL;
	//put to link
	link = get_thread_link( st );
	if(link){
#ifdef SCHED_FIFO
		if( *link ){
			THREAD* t2;
			t2 = *link;
			while( t2->sched_info.next )
				t2 = t2->sched_info.next;
			t2->sched_info.next = thr;
			info->pre = t2;
		}else{
			*link = thr;
		}
		info->next = NULL;
#else
//		抢占式。。。。
		info->next = *link;
		if( *link )
			(*link)->sched_info.pre = thr;
		*link = thr;
#endif
	}
	thr->state = st;
	//ok
	local_irq_restore( flags );
}

// 系统时间，单位：秒
extern time_t rtc_second;
//调度器获得时钟信号
void sched_clock()
{
	THREAD* thr;
	SCHEDULE_INFO* info;
	uint flags;
	//禁中断
	local_irq_save(flags);
	//处理等待一定时间的线程
	for( thr=tbox.wait; thr; thr=thr->sched_info.next ){
		thr->sched_info.clock -= ms;
		if( thr->sched_info.clock<=0 ){
			THREAD* pre = thr->sched_info.pre;
			sched_set_state( thr, TS_READY );
			if( pre )
				thr = pre;
			else 
				thr = tbox.wait;
			if(!thr) break;  //##important
		}
	}
	//允许中断
	local_irq_restore(flags);
	//获取当前线程
	thr = current_thread();
	//是否用户态线程
	if( !thr->kernel ){
		//用户空间的时间变化
		thr->thread_info->time = rtc_second;
	}
	//获取调度信息
	info = &thr->sched_info;
	--info->clock;
	if( info->clock <= 0 && sched_off==0 ){	//need scheduling
		schedule();
	}
}

//调度函数，取调度列表的下一个。
void schedule()
{
	THREAD* thr, *cur;
	uint flags;
	cur = current_thread();
	//禁止中断
	local_irq_save(flags);
	//next one
	thr = cur->sched_info.next;
	if(!thr || thr->state!=TS_READY)	//one round finished...
		thr = tbox.ready;
	if( thr ){
		thr->sched_info.clock = 1;
		if( thr != cur ){
			// 指定下一个线程
			tbox.next = thr;
//			kprintf("[%d]", thr->tid );
			// 非硬件中断下允许直接切换线程
			if( !cur->interrupted )
				switch_to( cur, thr );	//线程切换
		}
	}
	//back to me
	local_irq_restore(flags);
}


