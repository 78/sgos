// SGOS2
// Huang Guan
// 物理页面内存管理器

#include <sgos.h>
#include <arch.h>
#include <debug.h>
#include <string.h>
#include <process.h>
#include <multiboot.h>

// 初始化时计算一个有多少个物理页面，
// 然后每个页面用一个uchar来记录引用次数。
// 
// 一个字节记录1个页面的使用情况，
// 4GB内存需要。。。1MB
// 假设内核以及基础服务占去 0x00100000-0x00200000 1MB
// 那么剩下2MB，可以使用了。。。
// 0x00200000 - 0x00300000 分页信息存储区
// 0x00300000 - 0x00302000 内核页目录与页表区
// 0x00302000 - 0x00303000 内核栈
// 0x00303000 - 0x00400000 保留
// 要用空间换取效率的话，这里还是使用链表好。。。。
// 0xE0000000 - 0xE0400000 4MB来映射各个进程的页目录，每个进程占用4KB
// 0xBFC00000 - 0xC0800000 4MB用来映射进程各个页表

#define PAGE_INDEX_TO_PHYS_ADDR( i ) ( (uint)(i<<PAGE_SIZE_BITS) )
#define PHYS_ADDR_TO_PAGE_INDEX(addr) ((uint)addr>>PAGE_SIZE_BITS)

static uint	total_pages;	//总物理页面数
static uchar*	page_ref;	//page reference count
static uint	page_used;	//used page count
static uint	page_front;	//第一个可使用的页面
uint		kernel_page_dir;	//page dir for kernel
static uint	page_it = 0;	//for fast allocation

//页面异常调用函数
extern int pagefault_handler( int err_code, I386_REGISTERS* r );

//初始化分页
int page_init(uint mem_size)
{
	int i;
	PAGE_DIR* dir_entry, *te;
	if( mem_size == 0 ){
		KERROR("Sorry, no memory detected on this machine.");
	}
	//we initialized it in multiboot.S
	page_ref = (uchar*) (KERNEL_BASE+0x00200000);
	memsetd( page_ref, 0, (1<<20)>>2 ); //set 1MB zeros
	//计算物理页面总数
	total_pages = mem_size / PAGE_SIZE;
	//第一个可分配物理页面
	page_front = PHYS_ADDR_TO_PAGE_INDEX(0x00400000);
	page_used = page_front;
	//page_it指向一个可用的物理页面，为了快速分配使用。
	page_it = page_front;
	//设置isr
	isr_install( PAGEFAULT_INTERRUPT, (void*)pagefault_handler );
	//分配所有的共享页表 3G-4G  大概需要2MB
	kprintf("Allocating tables for kernel space.\n");
	dir_entry = (PAGE_DIR*)0xC0300000;
	//内核进程页目录
	kernel_page_dir = get_page_dir();
	// 分配内核3G-4G的页表，0xC0000000 - 0xC0400000 已分配，所以 +1
	for( i=768+1; i<1024; i++ ){
		dir_entry[i].v = get_phys_page();
		dir_entry[i].a.write = dir_entry[i].a.present = 1;
	}
	// 未分配的清0
	memsetd( dir_entry+1, 0, 768-1 );
	
	//映射内核空间的页目录的各页表，这样以后我们就可以很容易修改页表内容
	kprintf("Mapping tables for kernel process\n");
	i = PROC_PAGE_TABLE_MAP>>22;	//767
	dir_entry[i].v = 0x00300000|P_PRESENT|P_WRITE;
	reflush_pages();
	//为分配的页表清0
	memsetd( (void*)(PROC_PAGE_TABLE_MAP+769*PAGE_SIZE), 0, (1024-769)<<10 );
	//映射内核进程页目录到0xE0400000
	te = (PAGE_TABLE*)PROC_PAGE_TABLE_MAP + (kernel_page_dir>>12);
	te[0].v = 0x00300000|P_WRITE|P_PRESENT;
	
	//修改页表后更新cr3
	reflush_pages();
	return 0;
}

//if there's no page , return 0.
//## be careful multi-threading
uint get_phys_page()
{
	int i;
	uint eflags;
	if( page_used == total_pages ){
		PERROR("## no pages for allocation.");
		return 0;
	}
	//进入不可中断区域
	local_irq_save(eflags);
	if( page_it < page_front )
		page_it = page_front;
	//从page_it开始搜索
	for( i=page_it; i<total_pages; i++ ){
		if( !page_ref[i] ){
			page_ref[i]++;
			page_it = i+1;
			local_irq_restore(eflags);
			return PAGE_INDEX_TO_PHYS_ADDR(i);
		}
	}
	//如果找不到，再从头开始找
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

// 打印物理页面使用情况
void dump_phys_pages()
{
	uint i;
	for( i=page_front; i<total_pages; i++ ){
		if( page_ref[i] )
			kprintf("[%d:%d]", i, page_ref );
	}
	kprintf("\n");
}

//更新cr3（刷新页目录）
void load_page_dir(uint vir_addr)
{
	uint phys_addr = vir_to_phys(vir_addr);
 	__asm__ __volatile__("mov %0, %%eax"::"m"(phys_addr) );
	__asm__ __volatile__("mov %eax, %cr3");
}

//临时切换页目录，返回旧的页目录
uint switch_page_dir(uint vir_addr)
{
	uint old;
	__asm__ __volatile__ \
	("pushfl ; popl %0":"=g" (old): :"memory");
	load_page_dir( vir_addr );
	return old;
}

//由0xE0000000上的页目录虚拟地址得到物理页面的地址
uint vir_to_phys( uint vir_addr )
{
	PAGE_TABLE* te;
	te = (PAGE_TABLE*)PROC_PAGE_TABLE_MAP + (vir_addr>>12);
	return ((te->a.phys_addr)<<12);
}

//刷新当前进程的页目录
void reflush_pages()
{
	PROCESS* proc = current_proc();
	if( proc )
		load_page_dir( proc->page_dir );
	else
		__asm__ __volatile__("movl $0x00300000, %eax; movl %eax, %cr3");
}

//初始化页目录 
void init_page_dir( uint vir_addr )
{
	PAGE_DIR* dir_entry = (PAGE_DIR*)vir_addr;
	PAGE_DIR* kdir_entry = (PAGE_DIR*)kernel_page_dir;
	uint phys_addr;
	int i;
	// 复制内核3G-4G的页表，0xC0000000 - 0xFFFFFFFF
	memcpyd( dir_entry + 768, kdir_entry + 768, 1024-768 );
	// 未分配的清0
	memsetd( dir_entry, 0, 768 );
	//映射内核空间的页目录的各页表，这样以后我们就可以很容易修改页表内容
	//获取页目录的物理地址
	phys_addr = vir_to_phys( vir_addr );
	//设置映射
	i = PROC_PAGE_TABLE_MAP>>22;	//767
	dir_entry[i].v = phys_addr|P_PRESENT|P_WRITE;
	//修改页表后更新cr3
	reflush_pages();
}
