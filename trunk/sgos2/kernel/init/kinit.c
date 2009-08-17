#include <sgos.h>
#include <debug.h>
#include <arch.h>
#include <multiboot.h>
#include <thread.h>
#include <string.h>
#include <process.h>
#include <mm.h>

//内核初始化入口
extern PROCESS* cur_proc;
static multiboot_info_t* mbi;	//=multiboot information
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
		kprintf ("mmap_addr = 0x%x, mmap_length = 0x%x\n", 
			mbi->mmap_addr, mbi->mmap_length); 
		mbi->mmap_addr += KERNEL_BASE;
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
	//init module management.
	module_init();
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

//加载模块
static void kinit_process_start()
{
	PROCESS* proc;
	proc = current_proc();
	if( proc->environment ){
		//根据环境信息加载程序。
	}else if( proc->module_addr ){
		MODULE* mod;
		THREAD* main;
		if( loader_process( proc, proc->name, (uchar*)proc->module_addr, 
			0, &mod ) < 0 ){
			PERROR("##failed to load module %s", proc->name );
		}
		main = thread_create( proc, mod->entry_address );
		thread_resume( main );
	}
	thread_terminate(current_thread(), 0);
}

//线程0执行hlt指令，线程1继续内核初始化。
void kinit_resume()
{
	//create a halt thread
	THREAD* thr;
	thr = thread_create( current_proc(), (uint)kinit_halt );
	thread_resume( thr );
	
	//check module   内核需要加载的基本服务信息   
	if (CHECK_FLAG (mbi->flags, 3)) 
	{ 
		module_t *mod; 
		int i; 
		char* ext;
		kprintf ("mods_count = %d, mods_addr = 0x%x\n", 
			(int) mbi->mods_count, (int) mbi->mods_addr ); 
		mbi->mods_addr += KERNEL_BASE;
		for (i = 0, mod = (module_t *) mbi->mods_addr; 
			i < mbi->mods_count; i++, mod ++) {
			kprintf ("mod_start = 0x%x, mod_end = 0x%x, string = %s\n", 
				mod->mod_start, 
				mod->mod_end, 
				(char *) mod->string ); 
			mod->mod_start += KERNEL_BASE;
			mod->string += KERNEL_BASE;
			//check file extension
			ext = strrchr( (char*)mod->string, '.' );
			if( ext && strcmp(ext,".bxm")==0 ){
				//load it and share it 
				if( loader_process( current_proc(), mod->string, (uchar*)mod->mod_start, 
					1, NULL ) < 0 ){
					PERROR("##failed to load module %s", mod->string );
				}
			}else{
				PROCESS* proc;
				THREAD* thr;
				//创建进程
				PERROR("create process %s", mod->string );
				proc = process_create( current_proc(), NULL );
				proc->module_addr = mod->mod_start;
				strncpy( proc->name, (char*)mod->string, PROCESS_NAME_LEN-1 );
				//为该进程创建一个内核线程来加载模块
				thr = thread_create( proc, (uint)kinit_process_start );
				thread_resume( thr );
			}
		}
	} 
	
	//do something boring currently
	kprintf("display a point every second.\n");
	while(1){
		kprintf(".");
		thread_wait(1000 );
	}
}
