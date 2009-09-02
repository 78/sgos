#include <sgos.h>
#include <debug.h>
#include <arch.h>
#include <multiboot.h>
#include <thread.h>
#include <string.h>
#include <process.h>
#include <module.h>
#include <mm.h>
#include <loader.h>
#include <message.h>
#include <stdlib.h>
#include <namespace.h>

//内核初始化入口
extern PROCESS* cur_proc;
extern THREAD_BOX tbox;	//in sched.c
multiboot_info_t* mbi;	//=multiboot information

void kinit( uint boot_info )
{
	uint mem_size;
	//before we do something, we initialize some important vars
	cur_proc = NULL;	
	
	//get mbi
	mbi = (multiboot_info_t*)(KERNEL_BASE+boot_info);
	//init debugger as soon as possible.
	debug_init();
	
	// memory map  MBI提供的内存映射信息
	if ( CHECK_FLAG (mbi->flags, 6) ) 
	{
		memory_map_t *mmap; 
		mbi->mmap_addr += KERNEL_BASE;
		for ( mmap = (memory_map_t *) mbi->mmap_addr; 
			(uint) mmap < mbi->mmap_addr + mbi->mmap_length; 
			mmap = (memory_map_t *) ((uint) mmap + mmap->size + sizeof (mmap->size))
		){
			kprintf ("size = 0x%02x, base = 0x%04x%08x, len = 0x%04x%08x, type = 0x%x\n", 
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
		kprintf ("mem_total = %uMB\n", mem_size>>20 );
	}
	//init machine 
	machine_init();
	//page_dir init
	dir_init();
	//page management
	page_init( mem_size );
	//init kmalloc
	kmalloc_init();
	//init thread management.
	thread_init();
	//set running thread
	sched_init();
	//init process management.
	process_init( (char*)mbi->cmdline );
	//init module management.
	module_init();
	//init name space
	name_init();
	//set running thread
	//设置当前运行的线程
	tbox.running = current_proc()->thread;
	//启动线程
	start_threading();
	//never return here
	KERROR("##Warning: kernel not ready.");
}

//线程0执行hlt指令，线程1继续内核初始化。
void kinit_resume()
{
	THREAD* thrLoad;
	extern void load_base_services();
	thrLoad = thread_create( current_proc(), (uint)load_base_services, KERNEL_THREAD );
	thread_resume( thrLoad );
	//becomes an idle thread.
	//do something boring currently
	for(;;) halt();
}
