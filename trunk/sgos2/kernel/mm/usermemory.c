//user space memory bigblock allocator
//provide a lot of memory for running SGOS2 user process
//use 0x00000000 - 0x80000000  2GB  VIRTUAL_MEMORY

#include <sgos.h>
#include <bigblock.h>
#include <kd.h>
#include <mm.h>


// 分配指定地址的用户态空间内存
void*	MmAllocateUserMemoryAddress(KSpace* space, size_t addr, size_t siz, uint pattr, uint flag)
{
	KMemoryInformation* info;
	void *ptr;
	int ret;
	if( !space )
		return NULL;
	ASSERT( addr%PAGE_SIZE==0 && siz%PAGE_SIZE==0 );
	//change some mem info
	info = &space->MemoryInformation;
	info->UserMemoryAllocated += siz;
	if( info->UserMemoryAllocated > info->UserMemoryCapacity ){
		info->UserMemoryAllocated -= siz;
		PERROR("##user memory used out!");
		return NULL;
	}
	pattr |= PAGE_ATTR_USER|PAGE_ATTR_PRESENT;
	ptr = MmAllocateVirtualMemory( &space->VirtualMemory, addr, siz, pattr, flag );
	if( ptr==NULL ){
		info->UserMemoryAllocated -= siz;
		return NULL;
	}
	if( !(flag&ALLOC_VIRTUAL) && !(flag&ALLOC_LAZY) ){ //是否立刻分配物理地址
		uint mapFlag = MAP_ATTRIBUTE;
		if( flag&ALLOC_ZERO )
			mapFlag |= MAP_ZERO;
		ret = MmAcquireMultiplePhysicalPages( space, (size_t)ptr, siz, pattr, mapFlag );
		if( ret<0 ){ 
			MmFreeUserMemory(space, ptr);
			return NULL;
		}
	}
	return ptr;
}

// 分配用户态空间的内存
void*	MmAllocateUserMemory(KSpace* space, size_t siz, uint attr, uint flag)
{
	return MmAllocateUserMemoryAddress( space, 0, siz, attr, flag|ALLOC_RANDOM );
}

// 释放用户空间的内存
void	MmFreeUserMemory(KSpace* space, void* p)
{
	KMemoryInformation* info;
	size_t siz;
	uint state;
	if( space ){
		info = &space->MemoryInformation;
		//此时避免其他线程使用内存分配。
		TmSaveScheduleState(state);
		siz = MmFreeVirtualMemory( &space->VirtualMemory, p );
		MmReleaseMultiplePhysicalPages( space, (size_t)p, siz );
		TmRestoreScheduleState(state);
		info->UserMemoryAllocated -= siz;
		if( info->UserMemoryAllocated < 0 )
			info->UserMemoryAllocated = 0;
	}
}

