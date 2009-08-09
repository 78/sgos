//kernel memory allocator
//provide a lot of memory for running SGOS2 kernel
//use 0xC0400000 - 0xE0000000  508MB  VIRTUAL_MEMORY

#include <sgos.h>
#include <allocator.h>
#include <debug.h>
#include <process.h>
#include <mm.h>

allocator_t km_block;

void kmalloc_init()
{
	mm_init_block( &km_block, 
		0xC0400000,			//address
		0xE0000000-0xC0400000		//size
		);
	//
	PERROR("ok");
}

void*	kmalloc(size_t siz)
{
	PROCESS* proc;
	MEMORY_INFO* info;
	proc = current_proc();
	if( proc ){
		//change some mem info
		info = &proc->memory_info;
		info->kmem_size += siz;
		if( info->kmem_size > info->max_kmem ){
			info->kmem_size -= siz;
			return NULL;
		}
	}
	return mm_alloc( &km_block, siz );
}

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

int	kcheck_allocated(size_t addr)
{
	return mm_check_allocated( &km_block, addr );
}


