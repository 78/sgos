//kernel memory allocator
//provide a lot of memory for running SGOS2 kernel
//use 0xC0400000 - 0xE0000000  508MB  VIRTUAL_MEMORY

#include <sgos.h>
#include <allocator.h>
#include <kd.h>
#include <mm.h>

//Kernel Memory Information Pool
allocator_t km_block;

// 内核内存初始化
void MmInitializeKernelMemoryPool()
{
	mm_init_block( &km_block, 
		0xC0400000,			//address
		0xE0000000-0xC0400000		//size
		);
}

// 分配内核内存
void*	MmAllocateKernelMemory(size_t siz)
{
	KSpace* space;
	KMemoryInformation* info;
	void* ptr;
	space = MmGetCurrentSpace();
	//让siz按4字节对齐
	siz = (siz&3?(siz&(~3))+4:siz);
	if( space ){
		//change some mem info
		info = &space->MemoryInformation;
		info->KernelMemoryAllocated += siz;
		if( info->KernelMemoryAllocated > info->KernelMemoryCapacity ){
			info->KernelMemoryAllocated -= siz;
			return NULL;
		}
	}
	ptr = mm_alloc( &km_block, siz );
	return ptr;
}

// 释放内核内存
void	MmFreeKernelMemory(void* p)
{
	KSpace* space;
	KMemoryInformation* info;
	space = MmGetCurrentSpace();
	size_t siz;
	siz = mm_free( &km_block, p );
	if( space ){
		//change some mem info
		info = &space->MemoryInformation;
		info->KernelMemoryAllocated -= siz;
		if( info->KernelMemoryAllocated < 0 )
			info->KernelMemoryAllocated = 0;
	}
}

// 检查是否在内核分配的块中
int	MmIsKernelMemoryAllocated(size_t addr)
{
	return mm_check_allocated( &km_block, addr );
}

// 打印内核内存分配信息
void	MmDumpKernelMemory()
{
	mm_print_block(&km_block);
}
