//SGOS2:address space management

#include <sgos.h>
#include <arch.h>
#include <mm.h>
#include <kd.h>
#include <rtl.h>

KSpace* FirstSpace = NULL;		//初始地址空间
extern uint KernelPageDirectory;	//内核进程页目录
KSpace* CurrentSpace = NULL;		//当前地址空间
static int spaceId = 0;		//空间ID计数器

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
static void InitializeSpaceBasicInformation( KSpace* space )
{
	KMemoryInformation* mem_info;
	space->SpaceId = GenerateSpaceId();
	//sema_init( &space->semaphore );
	mem_info = &space->MemoryInformation;
	//当前地址空间中用户态内存
	mem_info->UserMemoryCapacity = 1<<30;		//1GB !!!
	//当前地址空间中内核态内存
	mem_info->KernelMemoryCapacity = 0x1000000;		//16MB !!!
	//用户态空间内存初始化。
	MmInitializeUserMemoryPool( space );
}

//第一个地址空间初始化
void MmInitializeSpaceManagement()
{
	KThread* init_thr;
	spaceId = 0;
	FirstSpace = (KSpace*)MmAllocateKernelMemory( sizeof(KSpace) );
	RtlZeroMemory( FirstSpace, sizeof(KSpace) );
	FirstSpace->Magic = SPACE_MAGIC;
	// use kernel page directory
	FirstSpace->PageDirectory = KernelPageDirectory;
	// restore basic information
	InitializeSpaceBasicInformation( FirstSpace );
	// set as current process
	CurrentSpace = FirstSpace;
	// create an init thread for the init process
	init_thr = TmCreateThread( CurrentSpace, (uint)MmInitializeSpaceManagement, KERNEL_THREAD ); //用process_init来标记是内核线程
	// set run time because no scheduler can be used at the present
	init_thr->ScheduleInformation.clock = 10;
	// set ready state!!
	TmSetThreadState( init_thr, TS_READY );
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
	space->PageDirectory = ArAllocatePageDirecotry();
	// 映射内核空间 
	ArInitializePageDirectory( space->PageDirectory ); //arch/*/page.c
	// restore basic information
	InitializeSpaceBasicInformation( space );
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
	KeBugCheck("not implemented.");
	return 0;
}


