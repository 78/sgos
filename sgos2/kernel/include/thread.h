#ifndef _THREAD_H_
#define _THREAD_H_

#include <sgos.h>
#include <arch.h>
#include <semaphore.h>
#include <message.h>
#include <queue.h>

#define THREAD_MAGIC	0xFF0B45FC
#define THREAD_KERNEL_STACK_SIZE (1024*15)
#define THREAD_STACK_SIZE (1<<20)	//1MB

struct PROCESS;
struct THREAD;

//线程状态
typedef enum THREAD_STATE{
	TS_INIT = 0,			//线程初始化中
	TS_READY,			//线程就绪，可能在running中，也可能是等待run
	TS_RUNNING,			//此状态未使用
	TS_PAUSED,			//线程暂停
	TS_SLEEP,			//线程休眠,一般是等待锁或者信号量
	TS_WAIT,			//线程等待某一个时间
	TS_DEAD				//线程死亡,等待回收.
}THREAD_STATE;

//调度信息
typedef struct SCHEDULE_INFO{
	//clock在线程状态为running时候,它是计算剩余运行时间,wait时候是等待运行时间
	int				clock;
	struct THREAD*			pre, *next;	//schedule link
	uint				cpu;		//running by which cpu??
}SCHEDULE_INFO;

// 内核线程结构体
typedef struct THREAD{
	//线程运行时堆栈指针，修改此变量位置需要调整interrupt.S的线程切换处
	uint				stack_pointer;	
	//线程ID,用来给开发者定位线程
	int				tid;
	//在修改线程资料时使用
	sema_t				semaphore;
	//线程正在睡眠时,sleepon指向等待唤醒的锁
	void*				sleepon;
	//拥有该线程的进程指针
	struct PROCESS*			process;
	//线程列表，仅进程内部
	struct THREAD*			pre, *next;	//thread link
	//线程标记，用来验证线程结构
	uint				magic;
	//thread state
	enum THREAD_STATE		state;	
	//schedule information such as I387
	struct ARCH_THREAD		arch;		
	/*run time, read or write information, message information*/
	void*				load_info;
	//Thread Information Block，用户态线程才有。
	THREAD_INFO*			thread_info;
	//消息队列
	queue_t				message_queue;
	//线程优先级 1~4
	int				priority;
	//调度信息,调度链表 时间片之类的.
	SCHEDULE_INFO			sched_info;
	//线程退出码
	uint				exit_code;		
	//线程入口
	uint				entry_address;	
	//线程运行时堆栈地址
	uint				stack_address;	
	//线程运行时堆栈大小
	uint				stack_size;	
	//判断是否是内核线程。
	uchar				kernel;		
	//是否在中断模式下。
	uchar				interrupted;
	//是否使用了数学协处理器
	uchar				used_math;
	//线程内核堆栈
	uchar				kernel_stack[THREAD_KERNEL_STACK_SIZE];	
}THREAD;

//these threads on the same state are linked by schedule link
typedef struct THREAD_BOX{
	//下面两个仅作指针使用，不构成链表。
	THREAD*			running;	//正在运行的线程
	THREAD*			next;		//下一个调度的线程
//--------------------我是可爱的分隔线----------------------------
	THREAD*			ready;		//就绪的线程
	THREAD*			sleep;		//睡眠的线程
	THREAD*			paused;		//挂起的线程
	THREAD*			dead;		//死亡的线程
	THREAD*			wait;		//等待的线程
	sema_t			semaphore;	//未用...
}THREAD_BOX;
extern THREAD_BOX	tbox;	//线程盒子

//函数定义
THREAD* current_thread();
THREAD* thread_create( struct PROCESS* proc, uint entry_addr );
int thread_terminate( THREAD* thr, int code );
int thread_wakeup( THREAD* thr );
int thread_resume( THREAD* thr );
int thread_suspend( THREAD* thr );
int thread_wait( uint ms );
int thread_sleep();
void thread_init();
THREAD* thread_get( int tid );
//sched.c
void schedule();
void sched_clock();
void sched_set_state( THREAD* thr, enum THREAD_STATE st );
void sched_init();
extern int sched_off;
#define sched_state_save(s) {s=sched_off;sched_off=1;}
#define sched_state_restore(s) sched_off=s

#endif
