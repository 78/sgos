//global memory bigblock allocator
//provide global memory for storing global informations
//use 0xE0000000 - 0xE0400000  4MB  VIRTUAL_MEMORY

#include <sgos.h>
#include <bigblock.h>
#include <kd.h>
#include <mm.h>

//Global Memory Information Pool
static bigblock_t gb_block;

// 全局空间内存初始化
void MmInitializeGlobalMemoryPool()
{
	bb_init_block( &gb_block, 
		GLOBAL_MEMORY_BEG,			//address
		GLOBAL_MEMORY_END-GLOBAL_MEMORY_BEG	//size
		);
}

// 分配全局空间内存
void*	MmAllocateGlobalMemory( size_t siz, uint pattr, uint flag)
{
	void *ptr;
	int ret;
	ASSERT( siz%PAGE_SIZE==0 );
	ptr = bb_alloc( &gb_block, siz );
	if( ptr==NULL )
		return NULL;
	if( !(flag&ALLOC_VIRTUAL) ){ //是否立刻分配物理地址
		ret = MmAcquireMultiplePhysicalPages( MmGetCurrentSpace(), (size_t)ptr, 
			siz, pattr|PAGE_ATTR_USER|PAGE_ATTR_ALLOCATED, MAP_ATTRIBUTE );
		if( ret<0 ){ 
			MmFreeGlobalMemory(ptr);
			return NULL;
		}
	}else{
		ArMapMultiplePages( &MmGetCurrentSpace()->PageDirectory, (size_t)ptr, 0, siz,
			PAGE_ATTR_USER|PAGE_ATTR_ALLOCATED|pattr, MAP_ADDRESS|MAP_ATTRIBUTE );
	}
	return ptr;
}

// 释放全局空间的内存
void	MmFreeGlobalMemory(void* p)
{
	size_t siz;
	if(!p)
		return;
	siz = bb_free( &gb_block, p );
	//释放物理地址
	MmReleaseMultiplePhysicalPages( MmGetCurrentSpace(), (size_t)p, siz );
}

//判断内存地址是否已分配
int	MmIsGlobalMemoryAllocated(size_t addr)
{
	return bb_check_allocated( &gb_block, addr );
}
