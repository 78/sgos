//global memory management
//provide global memory for storing global informations
//use 0xE0000000 - 0xE0400000  4MB  VIRTUAL_MEMORY

#include <sgos.h>
#include <kd.h>
#include <mm.h>

//Global Memory Structure
KVirtualMemory GlobalVirtualMemory;

// 全局空间内存初始化
void MmInitializeGlobalMemoryPool()
{
	MmInitializeVirtualMemory( &GlobalVirtualMemory, 
		GLOBAL_MEMORY_BEG,			//address
		GLOBAL_MEMORY_END	//size
		);
}

// 分配全局空间内存
void*	MmAllocateGlobalMemory( size_t siz, uint pattr, uint flag)
{
	void *ptr;
	int ret;
	ASSERT( siz%PAGE_SIZE==0 );
	pattr |= PAGE_ATTR_USER|PAGE_ATTR_PRESENT;
	ptr = MmAllocateVirtualMemory( &GlobalVirtualMemory, 0, siz, pattr, flag|ALLOC_RANDOM );
	if( ptr==NULL )
		return NULL;
	if( !(flag&ALLOC_VIRTUAL) && !(flag&ALLOC_LAZY) ){ //是否立刻分配物理地址
		ret = MmAcquireMultiplePhysicalPages( MmGetCurrentSpace(), (size_t)ptr, 
			siz, pattr, MAP_ATTRIBUTE );
		if( ret<0 ){ 
			MmFreeGlobalMemory(ptr);
			return NULL;
		}
	}
	return ptr;
}

// 释放全局空间的内存
void	MmFreeGlobalMemory(void* p)
{
	size_t siz;
	if(!p)
		return;
	siz = MmFreeVirtualMemory( &GlobalVirtualMemory, p );
	//释放物理地址
	MmReleaseMultiplePhysicalPages( MmGetCurrentSpace(), (size_t)p, siz );
}
