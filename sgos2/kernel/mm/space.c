//SGOS2:address space management

#include <sgos.h>
#include <arch.h>
#include <mm.h>
#include <kd.h>
#include <rtl.h>

extern KPageDirectory KernelPageDirectory;	//内核进程页目录
KSpace* 	FirstSpace = NULL;		//初始地址空间
KSpace* 	CurrentSpace = NULL;		//当前地址空间
static ushort 	spaceId = 0;		//空间ID计数器

// 产生一个地址空间的ID
static int GenerateSpaceId()
{
	int spid = spaceId;
	spaceId += 4;
	return spid;
}

//由ID获取地址空间的指针
KSpace* MmGetSpaceById( uint spid )
{
	KSpace* space;
	if( FirstSpace->SpaceId == spid )
		return FirstSpace;
	//all spaces are the direct childron of FirstSpace?? Absolutely not.
	//#Warning: need to rewrite this code
	for( space=FirstSpace->child; space; space=space->next ){
		if( space->SpaceId == spid )
			return space;
	}
	return NULL;
}

//设置地址空间基本信息
void MmInitializeSpaceBasicInformation( KSpace* space )
{
	KMemoryInformation* mem_info;
	space->SpaceId = GenerateSpaceId();
	//sema_init( &space->semaphore );
	mem_info = &space->MemoryInformation;
	//当前地址空间中用户态内存
	mem_info->UserMemoryCapacity = 1<<30;		//1GB !!!
	//当前地址空间中内核态内存
	mem_info->KernelMemoryCapacity = 0x01000000;		//16MB !!!
	MmInitializeVirtualMemory( &space->VirtualMemory, MB(1), KERNEL_BASE );
}

//第一个地址空间初始化
void MmInitializeSpaceManagement()
{
	static KSpace init_space;
	spaceId = 0;
	FirstSpace = (KSpace*)&init_space;
	RtlZeroMemory( FirstSpace, sizeof(KSpace) );
	FirstSpace->Magic = SPACE_MAGIC;
	// use kernel page directory
	FirstSpace->PageDirectory = KernelPageDirectory;
	// set as current process
	CurrentSpace = FirstSpace;
}

//返回当前地址空间
KSpace* MmGetCurrentSpace()
{
	KThread* thr=TmGetCurrentThread();
	if( thr ){
		return thr->Space;
	}else{
		return FirstSpace;
	}
}

//创建进程
KSpace* MmCreateSpace( KSpace* parent )
{
	KSpace* space;
	uint flags;
	if( !parent )
		return NULL;
	space = (KSpace*)MmAllocateKernelMemory( sizeof(KSpace) );
	if( space == NULL )
		return NULL;
	RtlZeroMemory( space, sizeof(KSpace) );
	// 标识
	space->Magic = SPACE_MAGIC;
	// allocate a PageDirectory
	if( (space->PageDirectory.VirtualAddress = (size_t)MmAllocateVirtualMemory( &KernelVirtualMemory, 
		0, PAGE_SIZE, PAGE_ATTR_WRITE, ALLOC_RANDOM) )==0 ){
		MmFreeKernelMemory( space );
		return NULL;
	}
	if( MmAcquireMultiplePhysicalPages( MmGetCurrentSpace(), space->PageDirectory.VirtualAddress, 
		PAGE_SIZE, PAGE_ATTR_WRITE, MAP_ATTRIBUTE ) < 0 ){
		MmFreeVirtualMemory( &KernelVirtualMemory, (void*)space->PageDirectory.VirtualAddress );
		MmFreeKernelMemory( space );
		return NULL;
	}
	if( ArQueryPageInformation( &FirstSpace->PageDirectory, 
		space->PageDirectory.VirtualAddress, &space->PageDirectory.PhysicalAddress, NULL )!=0 )
	{
		PERROR("ArQueryPageInformation failed. no physical page.");
	}
	// 映射内核空间 
	ArInitializePageDirectory( &space->PageDirectory ); //arch/*/page.c
	// restore basic information
	MmInitializeSpaceBasicInformation( space );
	// 设置用户
	space->UserId = parent->UserId;
	// 设置链表
	space->parent = parent;
	// 进入临界区
	ArLocalSaveIrq( flags );
	space->next = parent->child;
	if( parent->child )
		parent->child->prev = space;
	parent->child = space;
	// 离开临界区
	ArLocalRestoreIrq( flags );
	return space;
}

//销毁空间
int MmDestroySpace( KSpace* space )
{
	PERROR("MmDestroySpace not implemented.");
	return 0;
}


