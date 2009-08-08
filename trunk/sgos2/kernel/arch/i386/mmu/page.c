// SGOS2
// Huang Guan
// 物理页面内存管理器

#include <sgos.h>
#include <arch.h>
#include <debug.h>
#include <string.h>
#include <multiboot.h>

// 初始化时计算一个有多少个物理页面，
// 然后每个页面用一个ushort来记录引用次数。
// 
// 一个字节记录1个页面的使用情况，
// 4GB内存需要。。。2MB
// 假设内核以及基础服务占去 0x00100000-0x00200000 1MB
// 那么剩下2MB，可以使用了。。。
// 要用空间换取效率的话，这里还是使用链表好。。。。
// 0xE0000000 - 0xE0400000 4MB来映射各个进程的页目录，每个进程占用4KB
// 0xE0400000 - 0xE0800000 4MB用来映射内核各个页表

#define PAGE_INDEX_TO_PHYS_ADDR( i ) ( (uint)(i<<PAGE_SIZE_BITS) )
#define PHYS_ADDR_TO_PAGE_INDEX(addr) ((uint)addr>>PAGE_SIZE_BITS)

static uint	total_pages;
static ushort*	page_ref;	//page reference count
static uint	page_used;	//used page count
static uint	page_front;	//
uint		kernel_page_dir;	//page dir for kernel
static uint	page_it = 0;	//for fast allocation

extern int pagefault_handler( int err_code, I386_REGISTERS* r );
int page_init(uint mem_size)
{
	int i;
	PAGE_DIR* dir_entry, *table_entry;
	if( mem_size == 0 ){
		KERROR("Sorry, no memory detected on this machine.");
	}
	//we initialized it in multiboot.S
	page_ref = (ushort*) (KERNEL_BASE+0x00200000);
	memset( page_ref, 0, 2<<20 );
	total_pages = mem_size / PAGE_SIZE;
	page_front = PHYS_ADDR_TO_PAGE_INDEX(0x00400000);
	page_used = page_front;
	page_it = page_front;
	isr_install( PAGEFAULT_INTERRUPT, (void*)pagefault_handler );
	//分配所有的共享页表 3G-4G  大概需要2MB
	kprintf("Allocating tables for kernel space.\n");
	dir_entry = (PAGE_DIR*)0xC0010000;
	//内核进程页目录
	kernel_page_dir = get_page_dir();
	//临时使用一下0-4KB的空间
	table_entry = (PAGE_TABLE*)0xC0011000;
	// 0xC0000000 - 0xC0400000 已分配，所以 +1
	for( i=768+1; i<1024; i++ ){
		dir_entry[i].v = get_phys_page();
		dir_entry[i].a.write = dir_entry[i].a.present = 1;
		table_entry[0].v = dir_entry[i].v;
		reflush_pages();
		//在内核空间映射内核进程页目录
		memset( NULL, 0, PAGE_SIZE );
		if(i==896){
			PAGE_TABLE* te = NULL;
			te[0].v = 0x00010000|P_WRITE|P_PRESENT;
		}
	}
	//映射内核空间的页目录的各页表，这样以后我们就可以很容易修改页表内容
	kprintf("Mapping tables for kernel process\n");
	i = 0xE0400000>>22;
	dir_entry[i].v = 0x00010000|P_PRESENT|P_WRITE;
	//恢复该页
	table_entry[0].v = 0|P_WRITE|P_PRESENT;
	//修改页表后更新cr3
	reflush_pages();
	
	PERROR("ok");
	return 0;
}

//if there's no page , return 0.
//## be careful multi-threading
uint get_phys_page()
{
	register uint i;
	uint eflags;
	if( page_used == total_pages ){
		PERROR("## no pages for allocation.");
		return 0;
	}
	local_irq_save(eflags);
	if( page_it < page_front )
		page_it = page_front;
	for( i=page_it; i<total_pages; i++ ){
		if( !page_ref[i] ){
			page_ref[i]++;
			page_it = i+1;
			local_irq_restore(eflags);
			return PAGE_INDEX_TO_PHYS_ADDR(i);
		}
	}
	for( i=page_front; i<page_it; i++ ){
		if( !page_ref[i] ){
			page_ref[i]++;
			page_it = i+1;
			local_irq_restore(eflags);
			return PAGE_INDEX_TO_PHYS_ADDR(i);
		}
	}
	PERROR("## no pages for allocation.");
	local_irq_restore(eflags);
	return 0;
}

// free a page
void free_phys_page( uint addr )
{
	uint i;
	i = PHYS_ADDR_TO_PAGE_INDEX( addr );
	if( i < page_front || i >= total_pages ){
		PERROR("##wrong addr: 0x%X", addr );
		return;
	}
	if( !page_ref[i] ){
		PERROR("##trying to free free page: 0x%X", addr );
		return;
	}
	page_ref[i]--;
}

//
void dump_phys_pages()
{
	uint i;
	for( i=page_front; i<total_pages; i++ ){
		if( page_ref[i] )
			kprintf("[%d:%d]", i, page_ref );
	}
	kprintf("\n");
}

//更新cr3
void load_page_dir(uint phys_addr)
{
 	__asm__("mov %0, %%eax"::"m"(phys_addr) );
	__asm__("mov %eax, %cr3");
}

//刷新当前进程的页目录
void reflush_pages()
{
	load_page_dir( 0x00010000 );
}
