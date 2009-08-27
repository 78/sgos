//SGOS2 thread

#include <sgos.h>
#include <arch.h>
#include <thread.h>
#include <process.h>
#include <debug.h>
#include <string.h>
#include <mm.h>

// useless at the present
static int thread_id = 1;

// thread management.
void thread_init()
{
	thread_id = 1;
}

// just for fun :-)
static int generate_tid()
{
	int tid = thread_id;
	if( (++thread_id)%4 == 0 )
		++thread_id;
	return tid;
}

//初始化用户态线程信息块
static void init_thread_info( THREAD* thr )
{
	thr->thread_info = umalloc(thr->process, PAGE_ALIGN(sizeof(THREAD_INFO)));
	if( thr->thread_info ){
		THREAD_INFO* ti = thr->thread_info;
		//清0操作，以免用户获得错误信息
		memset( ti, 0, PAGE_ALIGN(sizeof(THREAD_INFO)) );
		//线程堆栈地址
		ti->stack_base = thr->stack_address;
		//线程堆栈大小
		ti->stack_size = thr->stack_size;
		//进程信息地址
		ti->process_info = thr->process->process_info;
		//指向自己
		ti->self = ti;
		//进程id
		ti->pid = thr->process->pid;
		ti->tid = thr->tid;
	}
}

//由线程ID获得线程结构指针
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
	//搜索所有进程
	for( proc=process_get(0)->child; proc; proc=proc->next ){
		for( thr=proc->thread; thr; thr=thr->next ){
			if( thr->tid == tid ){
				local_irq_restore(flags);
				return thr;
			}
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
	//分配线程内存空间
	thr = (THREAD*)kmalloc( sizeof(THREAD) );
	if( thr == NULL ){
		PERROR("## kernel memory used out!!");
		return NULL;
	}
	memset( thr, 0, sizeof(THREAD) );
	//线程标识
	thr->magic = THREAD_MAGIC;
	//初始化线程信号量
	sema_init( &thr->semaphore );
	//初始化消息队列
	message_init( thr );
	//调度信息
	sched = &thr->sched_info;
	//线程基本信息
	thr->tid = generate_tid();
	thr->process = proc;
	thr->state = TS_INIT;
	thr->priority = PRI_NORMAL;
	thr->entry_address = entry_addr;
	thr->stack_size = THREAD_STACK_SIZE;
	if( IS_USER_MEMORY( entry_addr ) ){//创建用户态线程？？
		thr->kernel = 0;
		//用户态线程堆栈
		thr->stack_address = (uint)umalloc( proc, thr->stack_size );
	}else{
		thr->kernel = 1;
	}
	//初始化用户态信息
	if( thr->kernel == 0 ){
		init_thread_info( thr );
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
	//return the pointer
	return thr;
}

//结束线程
int thread_terminate( THREAD* thr, int code )
{
	PROCESS* proc;
	proc = current_proc();
	//如果线程睡眠了，怎么办？
	//设置状态为死亡
	sched_set_state( thr, TS_DEAD );
	//线程退出码
	thr->exit_code = code;
	//
	message_destroy( thr );
	//Wakeup all related sleeping threads
	sema_destroy( &thr->semaphore );
	//向各系统服务发送线程结束消息
	
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

//挂起线程
int thread_suspend( THREAD* thr )
{
	sched_set_state( thr, TS_PAUSED );
	if( thr == current_thread() )
		schedule();
	return 0;
}

//睡眠线程。注意，中断情况下不可以睡眠。
int thread_sleep()
{
	sched_set_state( current_thread(), TS_SLEEP );
	schedule();
	ASSERT( current_thread()->state != TS_SLEEP );
	return 0;
}

//线程挂起等待一段时间
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
	ASSERT( cur->state != TS_WAIT );
	return 0;
}

//唤醒线程
int thread_wakeup( THREAD* thr )
{
	if( thr->state == TS_SLEEP ){
		sched_set_state( thr, TS_READY );
		return 0;
	}
	return -ERR_UNKNOWN;
}

//引发一个事件，让线程从内核态返回用户态后执行。
void thread_raise_event( THREAD* thr, uint addr )
{
	//这个有点复杂。。。
	die("not implemented.");
}
