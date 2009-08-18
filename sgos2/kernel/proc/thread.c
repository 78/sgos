//SGOS2 thread

#include <sgos.h>
#include <arch.h>
#include <thread.h>
#include <process.h>
#include <debug.h>
#include <string.h>

static uint thread_id = 1;

void thread_init()
{
	thread_id = 1;
}

static uint generate_tid()
{
	uint tid = thread_id;
	if( (++thread_id)%4 == 0 )
		++thread_id;
	return tid;
}

THREAD* thread_get( int tid )
{
	PROCESS* proc;
	THREAD* thr;
	uint flags;
	//看是否当前线程tid
	thr = current_thread();
	if( thr->tid == tid )
		return thr;
	//否则在当前进程中查找
	proc = current_proc();
	//是否需要禁中断呢？？
	local_irq_save(flags);
	for( thr=proc->thread; thr; thr=thr->next ){
		if( thr->tid == tid ){
			local_irq_restore(flags);
			return thr;
		}
	}
	local_irq_restore(flags);
	return NULL;
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
	thr->tid = generate_tid();
	thr->process = proc;
	thr->state = TS_INIT;
	thr->entry_address = entry_addr;
	thr->stack_size = THREAD_STACK_SIZE;
	if( IS_USER_MEMORY( entry_addr ) ){//创建用户态线程？？
		thr->kernel = 0;
		thr->stack_address = (uint)umalloc( proc, thr->stack_size );
	}else{
		thr->kernel = 1;
	//	thr->stack_address = (uint)kmalloc( thr->stack_size );//内核堆栈(内核线程不需要这个堆栈)
	}
	//初始化寄存器
	init_thread_regs( thr, current_thread(), NULL, entry_addr, 
		thr->stack_address + thr->stack_size );
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
int thread_terminate( THREAD* thr, int code )
{
	PROCESS* proc;
	proc = current_proc();
	sched_set_state( thr, TS_DEAD );
	//如果线程睡眠了
	if( thr->state == TS_SLEEP && thr->sleepon ){
		mutex_remove_thread(thr->sleepon, thr);
	}
	//向各服务发送线程结束消息
	
	//回收资源
	/*
	if( !thr->kernel ){//用户态线程？？
		ufree( proc, thr->stack_pointer );
	}*/
	schedule();
	return 0;
}

//等待一个线程结束
void thread_join( THREAD* thr )
{
	die("not implemented.");
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
	//关中断，禁止线程切换
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

//引发一个事件，让线程从内核态返回用户态后执行。
void thread_raise_event( THREAD* thr, uint addr )
{
	//这个有点复杂。。。
	die("not implemented.");
}
