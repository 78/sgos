#include <sgos.h>
#include <stdlib.h>
#include <kd.h>
#include <arch.h>
#include <multiboot.h>
#include <tm.h>
#include <mm.h>

//内核初始化入口
extern KSpace* CurrentSpace;
extern THREAD_BOX ThreadingBox;	//in sched.c
multiboot_info_t* mbi;	//=multiboot information

//启动微内核程序
void KeStartOs( size_t boot_info )
{
	size_t mem_size;
	//before we do something, we initialize some important vars
	CurrentSpace = NULL;	
	
	//get mbi
	mbi = (multiboot_info_t*)(KERNEL_BASE+boot_info);
	//init debugger as soon as possible.
	KdInitializeDebugger();
	// memory map  MBI提供的内存映射信息
	if ( CHECK_FLAG (mbi->flags, 6) ) 
	{
		memory_map_t *mmap; 
		mbi->mmap_addr += KERNEL_BASE;
		for ( mmap = (memory_map_t *) mbi->mmap_addr; 
			(uint) mmap < mbi->mmap_addr + mbi->mmap_length; 
			mmap = (memory_map_t *) ((uint) mmap + mmap->size + sizeof (mmap->size))
		){
			KdPrintf ("size = 0x%02x, base = 0x%04x%08x, len = 0x%04x%08x, type = 0x%x\n", 
				mmap->size, 
				mmap->base_addr_high, 
				mmap->base_addr_low, 
				mmap->length_high, 
				mmap->length_low, 
				mmap->type); 
		}
	}

	// get memory size 
	if ( CHECK_FLAG (mbi->flags, 0) ) {
		mem_size = (mbi->mem_upper + 1024)<<10;
		KdPrintf ("mem_total = %uMB\n", mem_size>>20 );
	}
	//init machine 
	ArInitializeSystem();
	//PageDirectory init
	ArInitializePageDirectoryManagement();
	//page management
	ArInitializePaging( mem_size );
	//init MmAllocateKernelMemory
	MmInitializeKernelMemoryPool();
	//init thread management.
	TmInitializeThreadManagement();
	//set running thread
	TmInitializeScheduler();
	//init space management.
	MmInitializeSpaceManagement();
	//init module management.
	//ExInitializeModuleManagement();
	//set running thread
	//设置当前运行的线程
	ThreadingBox.running = MmGetCurrentSpace()->FirstThread;
	//启动线程
	KdPrintf("Starting Multi-Threading Mode ...\n");
	ArStartThreading();
	//never return here
	KERROR("##Warning: kernel not ready.");
}

// 内核停止工作
void KeBugCheck(const char *s )
{
	KdPrint( (char*)s );
	ArLocalDisableIrq();
	while(1)
		halt();
}

//该线程定时出来点一下, 表示内核没有挂 - -
void KeKeepAlive()
{
	for(;;){
		TmSleepThread(5000);
		KdPrint(".");
	}
}

//线程0执行hlt指令，线程1继续内核初始化。
void KeResumeStart()
{
	KThread* thrLoad, *thrAlive;
	extern void KeLoadBaseServices();
	KdPrintf("Starting Services ...\n");
	thrLoad = TmCreateThread( MmGetCurrentSpace(), (size_t)KeLoadBaseServices, KERNEL_THREAD );
	TmResumeThread( thrLoad );
	thrAlive = TmCreateThread( MmGetCurrentSpace(), (size_t)KeKeepAlive, KERNEL_THREAD );
	TmResumeThread( thrAlive );
	//becomes an idle thread.
	TmSetThreadState( TmGetCurrentThread(), TS_IDLE );
	//do something boring currently
	for(;;) halt();
}
