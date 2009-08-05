#ifndef _MM_H_
#define _MM_H_

#include <sgos.h>


// kernel memory allocator
void	kmalloc_init();
void*	kmalloc(size_t siz);
void	kfree(void* p);
void*	kcalloc(size_t u, size_t c);
void* 	krealloc(void* p, size_t siz);

#endif

