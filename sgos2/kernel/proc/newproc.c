#include <sgos.h>
#include <arch.h>
#include <thread.h>
#include <process.h>
#include <module.h>
#include <debug.h>
#include <mm.h>
#include <string.h>
#include <loader.h>

//新进程的内核线程对进程空间进行初始化
void init_newproc()
{
	PROCESS* proc;
	MODULE* mod_api;
	THREAD* main;
	//当前进程
	proc = current_proc();	
	// 设置用户态信息
	// umalloc是最小分配8KB的
	proc->process_info = umalloc( proc, PAGE_ALIGN(sizeof(PROCESS_INFO)) );
	//设置进程信息
	if( proc->process_info ){ //复制数据到用户态
		PROCESS_INFO* pi = proc->process_info;
		memset( pi, 0, PAGE_ALIGN(sizeof(PROCESS_INFO)) );
		//复制进程名
		strcpy( pi->name, proc->name );
		//进程id
		pi->pid = proc->pid;
		pi->uid = proc->uid;
		pi->parent = proc->parent->pid;
		//复制环境变量
		pi->cmdline = umalloc( proc, PAGE_SIZE );
		pi->variables = umalloc( proc, ENV_VARIABLES_SIZE );
		if( pi->cmdline )
			memcpy( pi->cmdline, proc->environment->cmdline, PAGE_SIZE );
		if( pi->variables )
			memcpy( pi->variables, proc->environment->variables, ENV_VARIABLES_SIZE );
	}
	//获得api接口库
	mod_api = module_get_by_name( proc, "api.bxm");
	//创建进程用户主线程
	main = thread_create( proc, module_get_export_addr( mod_api, "__start_process" ), 0 );
	//设置主线程
	proc->main_thread = main;
	if( proc->process_info )
		proc->process_info->main_thread = (uint)main;
	thread_resume( main );
	thread_kill(current_thread(), 0);
}
