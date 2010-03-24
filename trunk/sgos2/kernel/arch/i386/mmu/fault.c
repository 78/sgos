//pagefault
//sgos2
//Xiaoxia 090806

#include <sgos.h>
#include <arch.h>
#include <kd.h>
#include <mm.h>
#include <tm.h>

//if in MmAllocateKernelMemory, we can not allow irqs
extern int in_allocator; 
//pagefault
//页面缺页、权限等异常处理
int pagefault_handler( int err_code, I386_REGISTERS* r )
{
	uint addr;
	KSpace* space;
	//被访问或修改的内存地址
	__asm__ __volatile__("movl %%cr2, %0" : "=r"( addr ) );
	space = MmGetCurrentSpace();

	/*
		If TmGetCurrentThread()==NULL, it means the kernel is sitll 
		initializing. If in MmAllocateKernelMemory, we should disable IRQ.
	*/
	if( TmGetCurrentThread() && !in_allocator )
		ArLocalEnableIrq();

	if( !(err_code&PAGE_ATTR_PRESENT ) )
		return MmHandleNonPresentPageFault( space, addr );
	else if(err_code&PAGE_ATTR_WRITE)
		return MmHandleReadOnlyPageFault( space, addr );
	else if(err_code&PAGE_ATTR_USER)
		PERROR("##user try to access kernel memory at 0x%X.", addr);
	//未能处理，则BSOD
	return 0;
}

