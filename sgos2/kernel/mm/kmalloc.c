//kernel memory allocator
//provide a lot of memory(1MB) for running SGOS2 kernel
//use 0xC0400000 - 0xE0000000  508MB  VIRTUAL_MEMORY

#include <sgos.h>
#include <allocator.h>
#include <debug.h>
#include <mm.h>

allocator_t km_block;

void kmalloc_init()
{
	mm_init_block( &km_block, 
		0xC0400000,				//address
		0xE0000000-0xC0400000	//size
		);
	//
	PERROR("ok");
}

void*	kmalloc(size_t siz)
{
	return mm_alloc( &km_block, siz );
}

void	kfree(void* p)
{
	return mm_free( &km_block, p );
}

void*	kcalloc(size_t u, size_t c)
{
	return mm_calloc( &km_block, u, c );
}

void* 	krealloc(void* p, size_t siz)
{
	return mm_realloc( &km_block, p, siz );
}


