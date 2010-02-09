#include <sgos.h>
#include <arch.h>
#include <tm.h>
#include <mm.h>
#include <kd.h>
/*
//新进程的内核线程对进程空间进行初始化
void init_newproc()
{
	KSpace* space;
	MODULE* mod_api;
	KThread* main;
	//当前进程
	space = MmGetCurrentSpace();	
	// 设置用户态信息
	// umalloc是最小分配8KB的
	space->process_info = MmAllocateUserMemory( space, PAGE_ALIGN(sizeof(PROCESS_INFO)) );
	//设置进程信息
	if( space->process_info ){ //复制数据到用户态
		PROCESS_INFO* pi = space->process_info;
		RtlZeroMemory( pi, PAGE_ALIGN(sizeof(PROCESS_INFO)) );
		//复制进程名
		strcpy( pi->name, space->name );
		//进程id
		pi->pid = space->pid;
		pi->UserId = space->UserId;
		pi->parent = space->parent->pid;
		//复制环境变量
		pi->cmdline = MmAllocateUserMemory( space, PAGE_SIZE );
		pi->variables = MmAllocateUserMemory( space, ENV_VARIABLES_SIZE );
		if( pi->cmdline )
			RtlCopyMemory( pi->cmdline, space->environment->cmdline, PAGE_SIZE );
		if( pi->variables )
			RtlCopyMemory( pi->variables, space->environment->variables, ENV_VARIABLES_SIZE );
	}
	//获得api接口库
	mod_api = module_get_by_name( space, "api.bxm");
	//创建进程用户主线程
	main = thread_create( space, module_get_export_addr( mod_api, "__start_process" ), 0 );
	//设置主线程
	space->main_thread = main;
	if( space->process_info )
		space->process_info->main_thread = (uint)main;
	thread_resume( main );
	TmTerminateThread(TmGetCurrentThread(), 0);
}
*/
