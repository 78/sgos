#include <sgos.h>
#include <debug.h>
#include <arch.h>
#include <multiboot.h>
#include <thread.h>
#include <process.h>
#include <mm.h>

//内核初始化入口
extern PROCESS* cur_proc;
void kinit( uint boot_info )
{
	uint mem_size;
	multiboot_info_t* mbi;
	//before we do something, we initialize some important vars
	cur_proc = NULL;
	
	//get mbi
	mbi = (multiboot_info_t*)boot_info;
	//init debugger as soon as possible.
	debug_init();
	
	//	check module        
	if (CHECK_FLAG (mbi->flags, 3)) 
	{ 
		module_t *mod; 
		int i; 
		kprintf ("mods_count = %d, mods_addr = 0x%x\n", 
			(int) mbi->mods_count, (int) mbi->mods_addr ); 
		for (i = 0, mod = (module_t *) mbi->mods_addr; 
			i < mbi->mods_count; i++, mod += sizeof (module_t)) 
			kprintf ("mod_start = 0x%x, mod_end = 0x%x, string = %s\n", 
				mod->mod_start, 
				mod->mod_end, 
				(char *) mod->string ); 
	} 
	// memory map
	if ( CHECK_FLAG (mbi->flags, 6) ) 
	{
		memory_map_t *mmap; 
		kprintf ("mmap_addr = 0x%x, mmap_length = 0x%x\n", 
			mbi->mmap_addr, mbi->mmap_length); 
		for ( mmap = (memory_map_t *) mbi->mmap_addr; 
			(uint) mmap < mbi->mmap_addr + mbi->mmap_length; 
			mmap = (memory_map_t *) ((uint) mmap + mmap->size + sizeof (mmap->size))
		){
			kprintf ("size = 0x%x, base_addr = 0x%x%x, length = 0x%x%x, type = 0x%x\n", 
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
	process_init();
	//启动线程
	start_threading();
	//never return here
	KERROR("##Warning: kernel not ready.");
}

//halt thread, did nothing, for cpu idle
static void kinit_halt()
{
	kprintf("halt thread started.\n");
	while(1)
		halt();
}

//线程0执行hlt指令，线程1继续内核初始化。
void kinit_resume()
{
	THREAD* thr;
	thr = thread_create( current_proc(), (uint)kinit_halt );
	sched_set_state( thr, TS_READY );
	kprintf("display a point every second.\n");
	while(1){
		kprintf(".");
		thread_wait(1000 );
	}
}
