//kernel memory allocator
//provide a lot of memory for running SGOS2 kernel
//use 0xC0400000 - 0xE0000000  508MB  VIRTUAL_MEMORY

#include <sgos.h>
#include <allocator.h>
#include <debug.h>
#include <process.h>
#include <mm.h>
#include <string.h>

//Kernel Memory Information Block
allocator_t km_block;

// 内核内存初始化
void kmalloc_init()
{
	mm_init_block( &km_block, 
		0xC0400000,			//address
		0xE0000000-0xC0400000		//size
		);
}

// 分配内核内存
void*	kmalloc(size_t siz)
{
	PROCESS* proc;
	MEMORY_INFO* info;
	void* ptr;
	proc = current_proc();
	//让siz按4字节对齐
	siz = (siz&3?(siz&(~3))+4:siz);
	if( proc ){
		//change some mem info
		info = &proc->memory_info;
		info->kmem_size += siz;
		if( info->kmem_size > info->max_kmem ){
			info->kmem_size -= siz;
			return NULL;
		}
	}
	ptr = mm_alloc( &km_block, siz );
	return ptr;
}

// 释放内核内存
void	kfree(void* p)
{
	PROCESS* proc;
	MEMORY_INFO* info;
	proc = current_proc();
	size_t siz;
	siz = mm_free( &km_block, p );
	if( proc ){
		//change some mem info
		info = &proc->memory_info;
		info->kmem_size -= siz;
		if( info->kmem_size < 0 )
			info->kmem_size = 0;
	}
}

// 检查是否在内核分配的块中
int	kcheck_allocated(size_t addr)
{
	return mm_check_allocated( &km_block, addr );
}


void	kdump()
{
	mm_print_block(&km_block);
}
