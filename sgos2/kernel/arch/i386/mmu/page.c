// SGOS2
// Xiaoxia
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

KPageDirectory	KernelPageDirectory;	//page dir for kernel

//页面异常调用函数
extern int pagefault_handler( int err_code, I386_REGISTERS* r );

//初始化分页
int ArInitializePaging(uint mem_size)
{
	int i;
	PAGE_DIR* dir_entry;
	if( mem_size == 0 ){
		KERROR("Sorry, no memory detected on this machine.");
	}
	//we initialized it in multiboot.S
	MmInitializePhysicalMemoryManagement( (KERNEL_BASE+0x00200000), (1<<20), mem_size );
	//设置isr
	ArInstallIsr( PAGEFAULT_INTERRUPT, (void*)pagefault_handler );
	//分配所有的共享页表 3G-4G  大概需要2MB
	// KdPrintf("Allocating tables for kernel space\n");
	dir_entry = (PAGE_DIR*)0xC0300000;
	//虚拟地址
	KernelPageDirectory.VirtualAddressInSpace0 = (size_t)dir_entry;
	//内核进程页目录
	KernelPageDirectory.PhysicalAddress = 0x00300000;
	// 分配内核3G-4G的页表，0xC0000000 - 0xC0400000 已分配，所以 +1
	for( i=768+1; i<1024; i++ ){
		dir_entry[i].v = MmGetPhysicalPage();
		void* p = (void*)ArMapTemporaryPage( dir_entry[i].v );
		RtlZeroMemory32( p, PAGE_SIZE );
		dir_entry[i].a.write = 
		dir_entry[i].a.user = 
		dir_entry[i].a.present = 1;
	}
	// 内核页目录未分配的清0
	RtlZeroMemory32( dir_entry, 768 );
	ArFlushPageDirectory();
	return 0;
}

//更新cr3（刷新页目录）
void ArLoadPageDirectory(struct KPageDirectory* dir)
{
 	__asm__ __volatile__("mov %0, %%eax"::"m"(dir->PhysicalAddress) );
	__asm__ __volatile__("mov %eax, %cr3");
}


//刷新当前页目录
void ArFlushPageDirectory()
{
	KSpace* space = MmGetCurrentSpace();
	if( space )
		ArLoadPageDirectory( &space->PageDirectory );
	else
		__asm__ __volatile__("movl $0x00300000, %eax; movl %eax, %cr3");
}

//刷新一个页表
void ArFlushPageTableEntry( uint virt_addr )
{
	__asm__ __volatile__("invlpg (%0)"::"r"(virt_addr):"memory" );
}

//初始化页目录 
void ArInitializePageDirectory( struct KPageDirectory* dir )
{
	PAGE_DIR* dir_entry = (PAGE_DIR*)dir->VirtualAddressInSpace0;
	PAGE_DIR* kdir_entry = (PAGE_DIR*)KernelPageDirectory.VirtualAddressInSpace0;
	// 复制内核3G-4G的页表，0xC0000000 - 0xFFFFFFFF
	RtlCopyMemory32( dir_entry + 768, kdir_entry + 768, 1024-768 );
	// 未分配的清0
	RtlZeroMemory32( dir_entry, 768 );
}
