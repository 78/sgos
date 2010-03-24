#ifndef _TM_H_
#define _TM_H_

#include <sgos.h>
#include <arch.h>
#include <rtl.h>
#include <ipc.h>

#define THREAD_MAGIC	0xFF0B45FC
#define THREAD_KERNEL_STACK_SIZE (1024*15)
#define THREAD_STACK_SIZE (1<<20)	//1MB

#ifndef INFINITE
#define INFINITE (-1)
#endif

struct KSpace;
struct KThread;

//线程状态
typedef enum THREAD_STATE{
	TS_INIT = 0,			//线程初始化中
	TS_READY,			//线程就绪，可能在running中，也可能是等待run
	TS_RUNNING,			//此状态未使用
	TS_PAUSED,			//线程暂停
	TS_SLEEP,			//线程休眠,一般是等待锁或者信号量
	TS_WAIT,			//线程等待某一个时间
	TS_DEAD,			//线程死亡,等待回收.
	TS_IDLE				//悠闲状态，CPU空闲的时候调用。
}THREAD_STATE;

//调度信息
typedef struct SCHEDULE_INFO{
	//clock在线程状态为running时候,它是计算剩余运行时间,wait时候是等待运行时间
	int				clock;
	struct KThread			*prev, *next;	//TmSchedule link
	int				cpu;		//running by which cpu??
}SCHEDULE_INFO;

// 内核线程结构体
typedef struct KThread{
	//线程运行时堆栈指针，修改此变量位置需要调整interrupt.S的线程切换处
	size_t				StackPointer;	
	//线程ID,用来给开发者定位线程
	int				ThreadId;
	//线程正在睡眠时,sleepon指向等待唤醒的锁
	void*				SleepLock;
	//拥有该线程的空间指针
	struct KSpace*			Space;
	//线程列表，仅当前地址空间
	struct KThread*			prev, *next;	//thread link
	//线程标记，用来验证线程结构
	uint				Magic;
	//thread state
	enum THREAD_STATE		ThreadState;	
	//TmSchedule information such as I387
	struct ArchThread		ArchitectureInformation;		
	/*run time, read or write information, message information*/
	void*				LoadInformation;
	//Thread Information Block，用户态线程才有。
	ThreadInformation*		UserModeThreadInformation;
	//消息队列
	KQueue				MessageQueue;
	//线程锁，锁定线程信息
	KSemaphore			Semaphore;
	//等待线程结束队列
	KSemaphore			JoinSemaphore;
	//线程优先级 1~4
	int				SchedulePriority;
	//调度信息,调度链表 时间片之类的.
	SCHEDULE_INFO			ScheduleInformation;
	//线程退出码
	uint				ExitCode;		
	//线程入口
	uint				EntryAddress;	
	//线程运行时堆栈地址
	uint				StackBase;	
	//线程运行时堆栈大小
	uint				StackLimit;	
	//判断是否是内核线程。
	t_8				IsKernelThread;		
	//是否在中断模式下。
	t_8				IsInterrupted;
	//是否使用了数学协处理器
	t_8				UsedMathProcessor;
	//是否在BIOS模式下
	t_8				InBiosMode;
	//线程内核堆栈
	t_8				KernelStack[THREAD_KERNEL_STACK_SIZE];	
}KThread;

//these threads on the same state are linked by TmSchedule link
typedef struct THREAD_BOX{
	//下面两个仅作指针使用，不构成链表。
	KThread*			running;	//正在运行的线程
	KThread*			next;		//下一个调度的线程
//--------------------我是可爱的分隔线----------------------------
	KThread*			ready;		//就绪的线程
	KThread*			sleep;		//睡眠的线程
	KThread*			paused;		//挂起的线程
	KThread*			dead;		//死亡的线程
	KThread*			wait;		//等待的线程
	KThread*			idle;		//悠闲的线程
}THREAD_BOX;
extern THREAD_BOX	ThreadingBox;	//调度线程盒子

#define USER_THREAD	0x0
#define BIOS_THREAD	0x1
#define KERNEL_THREAD	0x2

//函数定义
KThread* TmGetCurrentThread();
KThread* TmCreateThread( struct KSpace* space, size_t entry_addr, uint flag );
KThread* TmCreateAdvancedThread( struct KSpace* space, size_t entry_addr, size_t stack_limit, size_t stack_base, ThreadInformation* ti, int flag );
int TmTerminateThread( KThread* thr, uint code );
int TmWakeupThread( KThread* thr );
int TmResumeThread( KThread* thr );
int TmSuspendThread( KThread* thr );
int TmSleepThread( KThread* thr, uint ms );
void TmJoinThread( KThread* thr );
void TmInitializeThreadManagement();
KThread* TmGetThreadById( uint tid );
//sched.c
void TmSchedule();
void TmIncreaseTime();
void TmSetThreadState( KThread* thr, enum THREAD_STATE st );
void TmInitializeScheduler();
extern int CanSchedule;
#define TmSaveScheduleState(s) {s=CanSchedule;CanSchedule=0;}
#define TmRestoreScheduleState(s) CanSchedule=s

#endif
