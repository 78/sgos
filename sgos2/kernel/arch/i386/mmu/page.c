// SGOS2
// Huang Guan
// 物理页面内存管理器

#include <sgos.h>
#include <arch.h>
#include <kd.h>
#include <mm.h>
#include <rtl.h>
#include <multiboot.h>

// 初始化时计算一个有多少个物理页面，
// 然后每个页面用一个uchar来记录引用次数。
// 
// 一个字节记录1个页面的使用情况，
// 4GB内存需要。。。1MB
// 假设内核以及基础服务占去 0x00100000-0x00200000 1MB
// 那么剩下2MB，可以使用了。。。
// 0x00200000 - 0x00300000 分页信息存储区
// 0x00300000 - 0x00302000 内核页目录与前4MB页表区
// 0x00302000 - 0x00303000 内核栈
// 0x00303000 - 0x00304000 临时映射用的页表，具体见map.c
// 0x00304000 - 0x00400000 保留
// 要用空间换取效率的话，这里还是使用链表好。。。。
// 0xE0000000 - 0xE0400000 4MB来映射各个地址空间的页目录，每个地址空间占用4KB
// 0xBFC00000 - 0xC0800000 4MB用来映射某地址空间的页目录下的各个页表

#define PAGE_INDEX_TO_PHYS_ADDR( i ) ( (uint)(i<<PAGE_SIZE_BITS) )
#define PHYS_ADDR_TO_PAGE_INDEX(addr) ((uint)addr>>PAGE_SIZE_BITS)

static uint	total_pages;	//总物理页面数
static uchar*	page_ref;	//page reference count
static uint	page_used;	//used page count
static uint	page_front;	//第一个可使用的页面
uint		KernelPageDirectory;	//page dir for kernel
static uint	page_it = 0;	//for fast allocation

//页面异常调用函数
extern int pagefault_handler( int err_code, I386_REGISTERS* r );

//初始化分页
int ArInitializePaging(uint mem_size)
{
	int i;
	PAGE_DIR* dir_entry, *te;
	if( mem_size == 0 ){
		KERROR("Sorry, no memory detected on this machine.");
	}
	//we initialized it in multiboot.S
	page_ref = (uchar*) (KERNEL_BASE+0x00200000);
	RtlZeroMemory( page_ref, (1<<20) ); //set 1MB zeros
	//计算物理页面总数
	total_pages = mem_size / PAGE_SIZE;
	//第一个可分配物理页面
	page_front = PHYS_ADDR_TO_PAGE_INDEX(0x00400000);
	page_used = page_front;
	//page_it指向一个可用的物理页面，为了快速分配使用。
	page_it = page_front;
	//设置isr
	ArInstallIsr( PAGEFAULT_INTERRUPT, (void*)pagefault_handler );
	//分配所有的共享页表 3G-4G  大概需要2MB
	// KdPrintf("Allocating tables for kernel space\n");
	dir_entry = (PAGE_DIR*)0xC0300000;
	//内核进程页目录
	KernelPageDirectory = ArAllocatePageDirecotry();
	// 分配内核3G-4G的页表，0xC0000000 - 0xC0400000 已分配，所以 +1
	for( i=768+1; i<1024; i++ ){
		dir_entry[i].v = ArGetPhysicalPage();
		dir_entry[i].a.write = dir_entry[i].a.present = 1;
	}
	// 未分配的清0
	RtlZeroMemory32( dir_entry+1, 768-1 );
	
	//映射内核空间的页目录的各页表，这样以后我们就可以很容易修改页表内容
	KdPrintf("Mapping tables for kernel process\n");
	i = SPACE_PAGETABLE_BEG>>22;	//767
	dir_entry[i].v = 0x00300000|PAGE_ATTR_PRESENT|PAGE_ATTR_WRITE;
	ArFlushPageDirectory();
	//为分配的页表清0
	RtlZeroMemory32( (void*)(SPACE_PAGETABLE_BEG+769*PAGE_SIZE), (1024-769)<<10 );
	//映射内核进程页目录到0xE0400000
	te = (PAGE_TABLE*)SPACE_PAGETABLE_BEG + (KernelPageDirectory>>12);
	te[0].v = 0x00300000|PAGE_ATTR_WRITE|PAGE_ATTR_PRESENT;
	//取消页表映射
	dir_entry[i].v = 0;
	//修改页表后更新cr3
	ArFlushPageDirectory();
	return 0;
}

//if there's no page , return 0.
//## be careful multi-threading
uint ArGetPhysicalPage()
{
	int i;
	uint eflags;
	if( page_used == total_pages ){
		PERROR("## no pages for allocation.");
		return 0;
	}
	//进入不可中断区域
	ArLocalSaveIrq(eflags);
	if( page_it < page_front )
		page_it = page_front;
	//从page_it开始搜索
	for( i=page_it; i<total_pages; i++ ){
		if( !page_ref[i] ){
			page_ref[i]++;
			page_it = i+1;
			ArLocalRestoreIrq(eflags);
			return PAGE_INDEX_TO_PHYS_ADDR(i);
		}
	}
	//如果找不到，再从头开始找
	for( i=page_front; i<page_it; i++ ){
		if( !page_ref[i] ){
			page_ref[i]++;
			page_it = i+1;
			ArLocalRestoreIrq(eflags);
			return PAGE_INDEX_TO_PHYS_ADDR(i);
		}
	}
	PERROR("## no pages for allocation.");
	ArLocalRestoreIrq(eflags);
	return 0;
}

// free a page
void ArFreePhysicalPage( uint addr )
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
void ArDumpPhysicalPages()
{
	uint i;
	for( i=page_front; i<total_pages; i++ ){
		if( page_ref[i] )
			KdPrintf("[%d:%d]", i, page_ref );
	}
	KdPrintf("\n");
}

//更新cr3（刷新页目录）
void ArLoadPageDirectory(uint virt_addr)
{
	uint phys_addr = ArVirtualToPhysicalAddress(virt_addr);
 	__asm__ __volatile__("mov %0, %%eax"::"m"(phys_addr) );
	__asm__ __volatile__("mov %eax, %cr3");
}

//临时切换页目录，返回旧的页目录
uint ArLoadPageDirectoryTemporary(uint virt_addr)
{
	uint old;
	__asm__ __volatile__ \
	("pushfl ; popl %0":"=g" (old): :"memory");
	ArLoadPageDirectory( virt_addr );
	return old;
}

//由0xE0000000上的页目录虚拟地址得到物理页面的地址
uint ArVirtualToPhysicalAddress( uint virt_addr )
{
	PAGE_TABLE* te;
	te = (PAGE_TABLE*)SPACE_PAGETABLE_BEG + (virt_addr>>12);
	return ((te->a.physicalAddress)<<12);
}

//刷新当前页目录
void ArFlushPageDirectory()
{
	KSpace* space = MmGetCurrentSpace();
	if( space )
		ArLoadPageDirectory( space->PageDirectory );
	else
		__asm__ __volatile__("movl $0x00300000, %eax; movl %eax, %cr3");
}

//刷新一个页表
void ArFlushPageTableEntry( uint virt_addr )
{
	__asm__ __volatile__("invlpg %0"::"m"(virt_addr) );
}

//初始化页目录 
void ArInitializePageDirectory( uint virt_addr )
{
	PAGE_DIR* dir_entry = (PAGE_DIR*)virt_addr;
	PAGE_DIR* kdir_entry = (PAGE_DIR*)KernelPageDirectory;
	uint phys_addr;
	int i;
	// 复制内核3G-4G的页表，0xC0000000 - 0xFFFFFFFF
	RtlCopyMemory32( dir_entry + 768, kdir_entry + 768, 1024-768 );
	// 未分配的清0
	RtlZeroMemory32( dir_entry, 768 );
	//映射内核空间的页目录的各页表，这样以后我们就可以很容易修改页表内容
	//获取页目录的物理地址
	phys_addr = ArVirtualToPhysicalAddress( virt_addr );
	//设置映射
	i = SPACE_PAGETABLE_BEG>>22;	//767
	dir_entry[i].v = phys_addr|PAGE_ATTR_PRESENT|PAGE_ATTR_WRITE;
	//修改页表后更新cr3
	ArFlushPageDirectory();
}
