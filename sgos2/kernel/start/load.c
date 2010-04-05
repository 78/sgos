// sgos2 微内核 加载服务

#include <sgos.h>
#include <arch.h>
#include <multiboot.h>
#include <tm.h>
#include <kd.h>
#include <mm.h>
#include <ke.h>

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
			ArMapMultiplePages( &MmGetCurrentSpace()->PageDirectory, 0, 
				0, MB(1), PAGE_ATTR_PRESENT|PAGE_ATTR_WRITE, MAP_ADDRESS|MAP_ATTRIBUTE );
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
			//wait for the service to initialize...
			TmSleepThread(TmGetCurrentThread(), 100); 
		}
	}
	//加载结束。
	TmTerminateThread( TmGetCurrentThread(), 0 );
}
