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
// 4GB内存需要。。。2048KB
// 假设内核以及基础服务占去 0x00100000-0x00200000 1MB
// 那么剩下2MB，可以使用了。。。
// 要用空间换取效率的话，这里还是使用链表好。。。。

#define PAGE_INDEX_TO_PHYS_ADDR( i ) ( (uint)(i<<12) )
#define PHYS_ADDR_TO_PAGE_INDEX(addr) ((uint)addr>>12)

uint	total_pages;
ushort*	page_ref;	//page reference count
uint	page_used;	//used page count
uint	page_front = PHYS_ADDR_TO_PAGE_INDEX(0x00400000);	//

int page_init(uint mem_size)
{
	if( mem_size == 0 ){
		KERROR("Sorry, no memory detected on this machine.");
	}
    page_ref = (ushort*) 0xC0100000;
	total_pages = mem_size / PAGE_SIZE;
	page_used = page_front;
	PERROR("ok");
	return 0;
}

//if there's no page , return 0.
//## be careful multi-threading
uint get_phys_page()
{
	static uint it = 0;
	register uint i;
	if( page_used == total_pages ){
		PERROR("## no pages for allocation.");
		return 0;
	}
	if( it < page_front )
		it = page_front;
	for( i=it; i<total_pages; i++ ){
		if( !page_ref[i] ){
			page_ref[i]++;
			it = i+1;
			return PAGE_INDEX_TO_PHYS_ADDR(it);
		}
	}
	for( i=0; i<it; i++ ){
		if( !page_ref[i] ){
			page_ref[i]++;
			it = i+1;
			return PAGE_INDEX_TO_PHYS_ADDR(it);
		}
	}
	PERROR("## no pages for allocation.");
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

