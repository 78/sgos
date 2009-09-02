// sgos2 微内核 加载服务

#include <sgos.h>
#include <arch.h>
#include <multiboot.h>
#include <thread.h>
#include <debug.h>
#include <mm.h>
#include <string.h>
#include <loader.h>

//加载系统服务
static void start_service()
{
	PROCESS* proc;
	MODULE* mod, *mod_api;
	THREAD* main;
	//当前进程
	proc = current_proc();	
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
	main = thread_create( proc, module_get_export_addr( mod_api, "__start_process" ), 0 );
	//设置主线程
	proc->main_thread = main;
	if( proc->process_info )
		proc->process_info->main_thread = (uint)main;
	thread_resume( main );
	thread_kill(current_thread(), 0);
}

//加载基础服务
extern multiboot_info_t* mbi;
void load_base_services()
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
					proc = process_create( current_proc() );
					proc->module_addr = mod->mod_start;
					proc->module_size = mod->mod_end - mod->mod_start;
					strncpy( proc->name, (char*)mod->string, PROCESS_NAME_LEN-1 );
					//为该进程创建一个内核线程来加载模块
					thr = thread_create( proc, (uint)start_service, KERNEL_THREAD );
					thread_resume( thr );
				}else if( strcmp(ext,".com")==0 ){//16位文件,BIOSCALL所需
					//映射前1MB
					map_pages( current_proc()->page_dir, 0, 0, 1<<20, P_USER|P_WRITE );
					memcpy( (void*)0x10100, (void*)mod->mod_start, mod->mod_end-mod->mod_start );
				}
			}
		}
	}
	//加载
	kprintf("boot_dev: %x  cmdline: %s\n", mbi->boot_device, mbi->cmdline );
	//加载结束。
	thread_kill( current_thread(), 0 );
}
