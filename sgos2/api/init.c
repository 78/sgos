
#include <sgos.h>
#include <api.h>

//从fs里获得用户态线程信息块
ThreadInformation* GetCurrentThreadInformation()
{
	size_t addr;
	__asm__ __volatile__ (
		"movl %%fs:(0x18), %%eax"
		: "=a" (addr) : ); 
	return (ThreadInformation*) addr; \
}

ProcessInformation* GetCurrentProcessInformation()
{
	return GetCurrentThreadInformation()->ProcessInformation;
}
