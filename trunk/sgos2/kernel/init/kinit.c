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
	process_init();
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

//加载模块
static void kinit_process_start()
{
	PROCESS* proc;
	MODULE* mod, *mod_api;
	THREAD* main;
	proc = current_proc();	//当前进程
	if( proc->environment ){
		//根据环境信息加载程序。
	}else if( proc->module_addr ){
		//根据内存地址加载
		if( loader_process( proc, proc->name, (uchar*)proc->module_addr, 
			0, &mod ) < 0 ){
			PERROR("##failed to load module %s", proc->name );
		}
	}
	
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
		//主线程
		//程序入口
		proc->process_info->entry_address = mod->entry_address;
	}
	//获得api接口库
	mod_api = module_get_by_name( proc, "api.bxm");
	//创建进程主线程
	main = thread_create( proc, module_get_export_addr( mod_api, "_program_entry_" ), 0 );
	//设置主线程
	proc->main_thread = main;
	if( proc->process_info )
		proc->process_info->main_thread = (uint)main;
	thread_resume( main );
	thread_terminate(current_thread(), 0);
}


//线程0执行hlt指令，线程1继续内核初始化。
void kinit_resume()
{
	//check module   内核需要加载的基本服务信息   
	if (CHECK_FLAG (mbi->flags, 3)) 
	{ 
		module_t *mod; 
		int i; 
		char* ext;
		mbi->mods_addr += KERNEL_BASE;	//修正地址
		for (i = 0, mod = (module_t *) mbi->mods_addr; 
			i < mbi->mods_count; i++, mod ++) {
			//修正地址
			mod->mod_start += KERNEL_BASE;
			mod->mod_end += KERNEL_BASE;
			mod->string += KERNEL_BASE;
			//check file extension
			ext = strrchr( (char*)mod->string, '.' );
			if( ext ){
				if( strcmp(ext,".bxm")==0 ){	//动态链接库
					//load it and share it 
					if( loader_process( current_proc(), (char*)mod->string, (uchar*)mod->mod_start, 
						1, NULL ) < 0 ){
						PERROR("##failed to load module %s", mod->string );
					}
				}else if( strcmp(ext,".sym")==0 ){//符号表
					debug_set_symbol( mod->mod_start, mod->mod_end );
				}else if( strcmp(ext,".run")==0 ){//执行文件
					PROCESS* proc;
					THREAD* thr;
					//创建进程
					proc = process_create( current_proc(), NULL );
					proc->module_addr = mod->mod_start;
					proc->module_size = mod->mod_end - mod->mod_start;
					strncpy( proc->name, (char*)mod->string, PROCESS_NAME_LEN-1 );
					//为该进程创建一个内核线程来加载模块
					thr = thread_create( proc, (uint)kinit_process_start, KERNEL_THREAD );
					thread_resume( thr );
				}else if( strcmp(ext,".com")==0 ){//16位文件,BIOSCALL所需
					//映射前1MB
					map_pages( current_proc()->page_dir, 0, 0, 1<<20, P_USER|P_WRITE );
					memcpy( (void*)0x10100, (void*)mod->mod_start, mod->mod_end-mod->mod_start );
				}
			}
		}
	} 
	//becomes an idle thread.
	//do something boring currently
	for(;;) halt();
}
