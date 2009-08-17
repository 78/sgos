//SGOS2:kernel process management

#include <sgos.h>
#include <arch.h>
#include <process.h>
#include <debug.h>
#include <string.h>

PROCESS* init_proc = NULL;	//初始进程
extern uint kernel_page_dir;	//内核进程页目录
PROCESS* cur_proc = NULL;		//当前进程
static int process_id = 0;		//进程ID计数器

static int generate_pid()
{
	int pid = process_id;
	process_id += 4;
	return pid;
}

//设置进程基本信息
static void process_init_basicinfo( PROCESS* proc )
{
	MEMORY_INFO* mem_info;
	proc->pid = generate_pid();
	mutex_init( &proc->mutex );
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

//创建进程
PROCESS* process_create( PROCESS* parent, ENVIRONMENT* env )
{
	PROCESS* proc;
	uint flags;
	if( !parent )
		return NULL;
	proc = (PROCESS*)kmalloc( sizeof(PROCESS) );
	if( proc == NULL )
		return NULL;
	memset( proc, 0, sizeof(PROCESS) );
	// allocate a page_dir
	proc->page_dir = get_page_dir();
	// 映射内核空间 
	init_page_dir( proc->page_dir ); //arch/*/page.c
	// restore basic information
	process_init_basicinfo( proc );
	proc->user = parent->user;
	// 设置进程链表
	proc->parent = parent;
	// 进入临界区
	local_irq_save( flags );
	proc->next = parent->child;
	if( parent->child )
		parent->child->pre = proc;
	parent->child = proc;
	// 离开临界区
	local_irq_restore( flags );
	PERROR("ok");
	return proc;
}

//挂起进程
int process_suspend( PROCESS* proc )
{
	die("not implemented.");
}

//结束进程
int process_terminate( PROCESS* proc, int exit_code )
{
	die("not implemented.");
}

//启动进程
int process_resume( PROCESS* proc )
{
	die("not implemented.");
}


