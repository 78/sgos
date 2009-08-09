//SGOS2 thread

#include <sgos.h>
#include <arch.h>
#include <thread.h>
#include <process.h>
#include <debug.h>
#include <string.h>

static uint thread_id = 0;

void thread_init()
{
	thread_id = 0;
}

//创建一个线程
THREAD* thread_create( PROCESS* proc, uint entry_addr )
{
	THREAD* thr;
	SCHEDULE_INFO* sched;
	uint eflags;
	thr = (THREAD*)kmalloc( sizeof(THREAD) );
	if( thr == NULL ){
		die("## kernel memory used out!!");
	}
	memset( thr, 0, sizeof(THREAD) );
	//初始化mutex
	mutex_init( &thr->mutex );
	//调度信息
	sched = &thr->sched_info;
	//线程基本信息
	thr->id = thread_id++;
	thr->process = proc;
	thr->state = TS_INIT;
	thr->entry_address = entry_addr;
	thr->stack_pointer = (uint)kmalloc(1024*1024);	//test
	//初始化寄存器
	init_thread_regs( thr, current_thread(), NULL, entry_addr, thr->stack_pointer );
	//进行链表操作
	local_irq_save( eflags );
	//插队
	thr->next = proc->thread;
	if( proc->thread )
		proc->thread->pre = thr;
	proc->thread = thr;
	local_irq_restore( eflags );
	//添加到thread_box
	sched_set_state( thr, TS_PAUSED );
	//
	return thr;
}

//结束线程
int thread_terminate( THREAD* thr )
{
	sched_set_state( thr, TS_DEAD );
	return 0;
}

//等待一个线程结束
void thread_join( THREAD* thr )
{
}

//线程继续
int thread_resume( THREAD* thr )
{
	sched_set_state( thr, TS_READY );
	return 0;
}

int thread_suspend( THREAD* thr )
{
	sched_set_state( thr, TS_PAUSED );
	if( thr == current_thread() )
		schedule();
	return 0;
}

int thread_sleep()
{
	sched_set_state( current_thread(), TS_SLEEP );
	schedule();
	if( current_thread()->state == TS_SLEEP )
		KERROR("##impossible.");
	return 0;
}

int thread_wait( uint ms )
{
	THREAD* cur = current_thread();
	uint flags;
	local_irq_save( flags );
	cur->sched_info.clock = ms;
	//如果此时被切换了线程，则clock被更改了。。。
	sched_set_state( cur, TS_WAIT );
	local_irq_restore( flags );
	schedule();
	if( cur->state == TS_WAIT )
		KERROR("##impossible.");
	return 0;
}

int thread_wakeup( THREAD* thr )
{
	sched_set_state( thr, TS_READY );
	return 0;
}
