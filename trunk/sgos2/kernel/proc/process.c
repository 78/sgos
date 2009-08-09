//SGOS2:kernel process management

#include <sgos.h>
#include <arch.h>
#include <process.h>
#include <debug.h>
#include <string.h>

PROCESS* init_proc = NULL;	//初始进程
extern uint kernel_page_dir;	//内核进程页目录
PROCESS* cur_proc = NULL;		//当前进程
static uint process_id = 0;		//进程ID计数器

//设置进程基本信息
static void process_init_basicinfo( PROCESS* proc )
{
	MEMORY_INFO* mem_info;
	init_proc->id = process_id ++;
	mutex_init( &init_proc->mutex );
	mem_info = &proc->memory_info;
	//进程用户态内存
	mem_info->max_umem = 1<<30;		//1GB !!!
	//进程内核态内存
	mem_info->max_kmem = 0x1000000;		//16MB !!!
	//用户态空间内存初始化。
	umalloc_init( proc );
}

//第一个进程初始化
void process_init()
{
	THREAD* init_thr;
	init_proc = (PROCESS*)kmalloc( sizeof(PROCESS) );
	memset( init_proc, 0, sizeof(PROCESS) );
	// init process name
	strcpy( init_proc->name, "init" );
	// use kernel page directory
	init_proc->page_dir = kernel_page_dir;
	// restore basic information
	process_init_basicinfo( init_proc );
	// set as current process
	cur_proc = init_proc;
	// create an init thread for the init process
	init_thr = thread_create( cur_proc, (uint)0 );
	// set run time because no scheduler can be used at the present
	init_thr->sched_info.clock = 10;
	// set ready state!!
	sched_set_state( init_thr, TS_READY );
	PERROR("ok");
}

//返回当前进程
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


