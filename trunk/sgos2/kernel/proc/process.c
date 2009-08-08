//SGOS2:kernel process management

#include <sgos.h>
#include <arch.h>
#include <process.h>
#include <debug.h>
#include <string.h>

PROCESS* init_proc;
extern uint kernel_page_dir;
PROCESS* cur_proc;


void process_init()
{
	THREAD* init_thr;
	init_proc = (PROCESS*)kmalloc( sizeof(PROCESS) );
	memset( init_proc, 0, sizeof(PROCESS) );
	init_proc->id = 0;
	strcpy( init_proc->name, "init" );
	init_proc->page_dir = kernel_page_dir;
	mutex_init( &init_proc->mutex );
	cur_proc = init_proc;
	init_thr = thread_create( cur_proc, (uint)0 );
	init_thr->sched_info.clock = 10;
	sched_set_state( init_thr, TS_READY );
}

PROCESS* current_proc()
{
	return cur_proc;
}

int process_create()
{
}

int process_suspend()
{
}

int process_terminate()
{
}

int process_sleep()
{
}

int process_wakeup()
{
}


