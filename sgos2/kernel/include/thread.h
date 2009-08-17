#ifndef _THREAD_H_
#define _THREAD_H_

#include <sgos.h>
#include <arch.h>
#include <mutex.h>
#include <message.h>

#define THREAD_KERNEL_STACK_SIZE (1024*7)
#define THREAD_STACK_SIZE (1<<20)	//1MB

struct PROCESS;
struct THREAD;

typedef enum THREAD_STATE{
	TS_INIT = 0,		//线程初始化中
	TS_READY,			//线程就绪，可能在running中，也可能是等待run
	TS_RUNNING,			//此状态未使用
	TS_PAUSED,			//线程暂停
	TS_SLEEP,			//线程休眠,一般是等待锁或者信号量
	TS_WAIT,			//线程等待某一个时间
	TS_DEAD				//线程死亡,等待回收.
}THREAD_STATE;

typedef struct SCHEDULE_INFO{
	//clock在线程状态为running时候,它是计算剩余运行时间,wait时候是等待运行时间
	int					clock;		
	struct THREAD*		pre, *next;	//schedule link
	uint				cpu;		//running by which cpu??
}SCHEDULE_INFO;

typedef struct THREAD{
	//线程ID,用来给开发者定位线程
	uint				tid;
	//在修改线程资料时,一般要lock下面的mutex
	mutex_t				mutex;
	//线程正在睡眠时,sleepon指向等待唤醒的mutex
	mutex_t*			sleepon;
	//拥有该线程的进程指针
	struct PROCESS*			process;
	struct THREAD*			pre, *next;	//thread link
	enum THREAD_STATE		state;		//thread state
	struct ARCH_THREAD		arch;		//schedule information
	//run time, clock, read or write information, message information
	void*				information;	//某些信息,暂未用
	MESSAGE_QUEUE			message_queue;		//消息链
	SCHEDULE_INFO			sched_info;		//调度信息,调度链表 时间片之类的.
	uint				exit_code;		//线程退出码
	uint				entry_address;	//线程入口
	uint				stack_address;	//线程运行时堆栈地址
	uint				stack_size;	//线程运行时堆栈大小
	uint				stack_pointer;	//线程运行时堆栈指针
	uchar				kernel;		//判断是否是内核线程。
	uchar				kernel_stack[THREAD_KERNEL_STACK_SIZE];	//线程中断时堆栈
}THREAD;

//these threads on the same state are linked by schedule link
typedef struct THREAD_BOX{
	THREAD*			running;	//正在运行的线程
	THREAD*			ready;		//就绪的线程
	THREAD*			sleep;		//睡眠的线程
	THREAD*			paused;		//挂起的线程
	THREAD*			dead;		//死亡的线程
	THREAD*			wait;		//等待的线程
	mutex_t			mutex;		//未用...
}THREAD_BOX;

THREAD* current_thread();
THREAD* thread_create( struct PROCESS* proc, uint entry_addr );
int thread_terminate( THREAD* thr, int code );
int thread_wakeup( THREAD* thr );
int thread_wait( uint ms );
int thread_sleep();
void thread_init();
//sched.c
void schedule();
void sched_clock();
void sched_set_state( THREAD* thr, enum THREAD_STATE st );


#endif
