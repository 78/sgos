
#include <sgos.h>
#include <api.h>

//从fs里获得用户态线程信息块
THREAD_INFO* thread_info()
{
	size_t addr;
	__asm__ __volatile__ (
		"movl %%fs:(0x18), %%eax"
		: "=a" (addr) : ); 
	return (THREAD_INFO*) addr; \
}

PROCESS_INFO* process_info()
{
	return thread_info()->process_info;
}

//进程主线程用户态入口点
void _program_entry_()
{
	int (*func)();
	THREAD_INFO* ti;
	PROCESS_INFO* pi;
	//获得线程信息块
	ti = thread_info();
	//获得进程信息块
	pi = ti->process_info;
	//获得运行地址
	func = (void*)pi->entry_address;
	sys_thread_exit(func());
}
