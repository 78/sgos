#ifndef _MM_H_
#define _MM_H_

#include <sgos.h>
#include <process.h>

// kernel memory allocator
void	kmalloc_init();
void*	kmalloc(size_t siz);
int	kcheck_allocated(size_t addr);
void	kfree(void* p);

// user space memory allocator
void	umalloc_init(PROCESS*);
void*	umalloc(PROCESS*, size_t siz);
void*	umalloc_ex(PROCESS*, size_t addr, size_t siz);
int	ucheck_allocated(PROCESS*, size_t addr );
void	ufree(PROCESS*, void* p);

#endif

