#ifndef _THREAD_H_
#define _THREAD_H_

#include <sgos.h>
#include <arch.h>
#include <mutex.h>
#define THREAD_KERNEL_STACK_SIZE 2048

struct PROCESS;
struct THREAD;

typedef enum THREAD_STATE{
	TS_INIT = 0,
	TS_READY,
	TS_RUNNING,
	TS_PAUSED,
	TS_SLEEP,
	TS_WAIT,
	TS_DEAD
}THREAD_STATE;

typedef struct SCHEDULE_INFO{
	int					clock;
	struct THREAD*		pre, *next;	//schedule link
	uint				cpu;		//running by which cpu??
}SCHEDULE_INFO;

typedef struct THREAD{
	uint				id;
	mutex_t				mutex;
	mutex_t*			sleepon;
	struct PROCESS*		process;
	struct THREAD*		pre, *next;	//thread link
	enum THREAD_STATE	state;
	struct ARCH_THREAD	arch;		//schedule information
	//run time, clock, read or write information, message information
	void*				information;
	void*				messsage;
	SCHEDULE_INFO		sched_info;
	uint				exit_code;
	uint				entry_address;
	uint				stack_pointer;
	uchar				kernel_stack[THREAD_KERNEL_STACK_SIZE];
}THREAD;

//these threads on the same state are linked by schedule link
typedef struct THREAD_BOX{
	THREAD*			running;
	THREAD*			ready;
	THREAD*			sleep;
	THREAD*			paused;
	THREAD*			dead;
	THREAD*			wait;
	mutex_t			mutex;
}THREAD_BOX;

THREAD* current_thread();
THREAD* thread_create( struct PROCESS* proc, uint entry_addr );
int thread_terminate( THREAD* thr );
int thread_wakeup( THREAD* thr );
int thread_wait( uint ms );
int thread_sleep();
//sched.c
void schedule();
void sched_clock();
void sched_set_state( THREAD* thr, enum THREAD_STATE st );


#endif
