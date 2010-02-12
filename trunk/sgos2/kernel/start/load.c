// sgos2 微内核 加载服务

#include <sgos.h>
#include <arch.h>
#include <multiboot.h>
#include <tm.h>
#include <kd.h>
#include <mm.h>
#include <ke.h>

//加载系统服务
/*
static void start_service()
{
	KSpace* space;
	MODULE* mod, *mod_api;
	KThread* main;
	//当前
	space = MmGetCurrentSpace();	
	if( space->environment ){
		//根据环境信息加载程序。
	}else if( space->module_addr ){
		//根据内存地址加载
		if( loader_process( space, space->name, (t_8*)space->module_addr, 
			0, &mod ) < 0 ){
			PERROR("##failed to load module %s", space->name );
		}
	}
	
	// 设置用户态信息
	// umalloc是最小分配8KB的
	space->process_info = MmAllocateUserMemory( space, PAGE_ALIGN(sizeof(PROCESS_INFO)) );
	//设置信息
	if( space->process_info ){ //复制数据到用户态
		PROCESS_INFO* pi = space->process_info;
		RtlZeroMemory( pi, 0, PAGE_ALIGN(sizeof(PROCESS_INFO)) );
		//复制进程名
		strcpy( pi->name, space->name );
		//进程id
		pi->pid = space->pid;
		pi->UserId = space->UserId;
		pi->parent = space->parent->pid;
		//主线程
		//程序入口
		space->process_info->entry_address = mod->entry_address;
	}
	//获得api接口库
	mod_api = module_get_by_name( space, "api.bxm");
	//创建进程主线程
	main = thread_create( space, module_get_export_addr( mod_api, "__start_process" ), 0 );
	//设置主线程
	space->main_thread = main;
	if( space->process_info )
		space->process_info->main_thread = (uint)main;
	thread_resume( main );
	TmTerminateThread(TmGetCurrentThread(), 0);
}
*/

// 内核下创建一个服务
int KeCreateBaseService(const char* srvName, size_t addr, size_t siz )
{
	const char* ext;
	int ret;
	//check file extension
	for( ext=srvName; *ext && *ext!='.'; ext++ )
		;
	if( *ext ){
		if( ext[1]=='e' && ext[2]=='x' && ext[3]=='e' ){	//exe
			//it's a PE Executable
			KSpace* space;
			size_t entry;
			//创建地址空间
			KdPrintf("Found Pe Executable: %s\n", srvName );
			space = MmCreateSpace( MmGetCurrentSpace() );
			if( (ret=KeLoadPeExecutable( space, addr, siz, &entry )) != 0 ){
				PERROR("Load failed: %s, ret=%d\n", srvName, ret );
				return ret;
			}
			KThread* srvThread;
			srvThread = TmCreateThread( space, entry, USER_THREAD );
			TmResumeThread( srvThread );
		}else if( ext[1]=='c' && ext[2]=='o' && ext[3]=='m' ){//16位文件,BIOSCALL所需
			//映射前1MB
			KdPrintf("Installed bioscall service program at 0x%X\n", 0x10100 );
			MmAllocateUserMemoryAddress(MmGetCurrentSpace(), 0x0, PAGE_SIZE, PAGE_ATTR_WRITE, ALLOC_VIRTUAL);
			ArMapMultiplePages( &MmGetCurrentSpace()->PageDirectory, 0, 
				0, 1<<20, PAGE_ATTR_PRESENT|PAGE_ATTR_WRITE, MAP_ADDRESS|MAP_ATTRIBUTE );
			RtlCopyMemory( (void*)0x10100, (void*)addr, siz );
		}
	}
	return 0;
}

//加载基础服务
extern multiboot_info_t* mbi;
void KeLoadBaseServices()
{
	KdPrintf("KeLoadBaseServices.\n");
	//check module   内核需要加载的基本服务信息   
	if (CHECK_FLAG (mbi->flags, 3)) 
	{ 
		module_t *mod; 
		int i; 
		mbi->mods_addr += KERNEL_BASE;	//修正地址
		for (i = 0, mod = (module_t *) mbi->mods_addr; 
			i < mbi->mods_count; i++, mod ++) {
			//修正地址
			mod->mod_start += KERNEL_BASE;
			mod->mod_end += KERNEL_BASE;
			mod->string += KERNEL_BASE;
			KeCreateBaseService( (char*)mod->string, mod->mod_start, 
				mod->mod_end-mod->mod_start );
		}
	}
	//加载结束。
	TmTerminateThread( TmGetCurrentThread(), 0 );
}
