//physical memory

#include <sgos.h>
#include <kd.h>
#include <mm.h>

#define PAGE_INDEX_TO_PHYS_ADDR( i ) ( (uint)(i<<PAGE_SIZE_BITS) )
#define PHYS_ADDR_TO_PAGE_INDEX(addr) ((uint)addr>>PAGE_SIZE_BITS)

static uint	total_pages;	//总物理页面数
static uchar*	page_ref;	//page reference count
static uint	page_used;	//used page count
static uint	page_front;	//第一个可使用的页面
static uint	page_it = 0;	//for fast allocation

void MmInitializePhysicalMemoryManagement( size_t bitmap_start, size_t bitmap_size, size_t mem_size )
{
	page_ref = (uchar*) bitmap_start;
	RtlZeroMemory( page_ref, bitmap_size ); //set 1MB zeros
	//计算物理页面总数
	total_pages = mem_size / PAGE_SIZE;
	//第一个可分配物理页面
	page_front = PHYS_ADDR_TO_PAGE_INDEX(0x00400000);
	page_used = page_front;
	//page_it指向一个可用的物理页面，为了快速分配使用。
	page_it = page_front;
}
	
//if there's no page , return 0.
//## be careful multi-threading
uint MmGetPhysicalPage()
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
	ArLocalRestoreIrq(eflags);
	PERROR("## no pages for allocation.");
	return 0;
}

// free a page
void MmFreePhysicalPage( uint addr )
{
	uint i;
	if( !addr )
		return;
	i = PHYS_ADDR_TO_PAGE_INDEX( addr );
	if( i < page_front || i >= total_pages ){
//		PERROR("##wrong addr: 0x%X", addr );
		return;
	}
	if( !page_ref[i] ){
		PERROR("##trying to free free page: 0x%X", addr );
		return;
	}
	page_ref[i]--;
}

void IncreasePhysicalPageReference( uint addr )
{
	uint i;
	i = PHYS_ADDR_TO_PAGE_INDEX( addr );
	if( i < page_front || i >= total_pages ){
		PERROR("##wrong addr: 0x%X", addr );
		return;
	}
	if( !page_ref[i] ){
		PERROR("##trying to increase the reference of a free page: 0x%X", addr );
		return;
	}
	page_ref[i]++;
}

ushort GetPhysicalPageReference( uint addr )
{
	uint i;
	i = PHYS_ADDR_TO_PAGE_INDEX( addr );
	if( i < page_front || i >= total_pages ){
		PERROR("##wrong addr: 0x%X", addr );
		return 0;
	}
	return page_ref[i];
}

// 打印物理页面使用情况
void MmDumpPhysicalPages()
{
	uint i;
	for( i=page_front; i<total_pages; i++ ){
		if( page_ref[i] )
			KdPrintf("[%d:%d]", i, page_ref );
	}
	KdPrintf("\n");
}

// 为虚拟地址申请多个物理页面
int	MmAcquireMultiplePhysicalPages( KSpace* space, size_t addr, size_t siz, uint attr, uint flag)
{
	uint count;
	count = siz >> PAGE_SIZE_BITS;
	for( ; count; count--, addr+=PAGE_SIZE ){
		if( MmAcquirePhysicalPage( space, addr, attr, flag )!=0 )
			return siz-count*PAGE_SIZE;
	}
	return siz;
}

// 为虚拟地址申请多个物理页面
void	MmReleaseMultiplePhysicalPages( KSpace* space, size_t addr, size_t siz)
{
	uint count;
	count = siz >> PAGE_SIZE_BITS;
	for( ; count; count--, addr+=PAGE_SIZE )
		MmReleasePhysicalPage( space, addr );
}

// 为虚拟地址申请一个物理页面
int	MmAcquirePhysicalPage( KSpace* space, size_t addr, uint attr, uint flag)
{
	// 分配物理页面
	uint phys_addr = MmGetPhysicalPage();
	if( !phys_addr )
		return -ERR_NOMEM;
	//映射到分配的物理地址去。
	ArMapOnePage( &space->PageDirectory, addr, phys_addr, attr|PAGE_ATTR_PRESENT, MAP_ADDRESS|flag );
	return 0;
}

// 释放虚拟地址上的物理页面
void	MmReleasePhysicalPage( KSpace* space, size_t addr)
{
	uint phys_addr, attr;
	if( ArQueryPageInformation( &space->PageDirectory, addr, &phys_addr, &attr ) == 0 ){
		ArMapOnePage( &space->PageDirectory, addr, 0, 0, MAP_ATTRIBUTE );
		if( attr & PAGE_ATTR_PRESENT )
			MmFreePhysicalPage( phys_addr );
	}
}

int MmSwapPhysicalPage( KSpace* destSp, size_t dest_addr, KSpace* srcSp, size_t src_addr, uint flag )
{
	size_t paS = 0, paD = 0;
	uint atS = 0, atD = 0;
	ArQueryPageInformation( &destSp->PageDirectory, dest_addr, &paD, &atD );
	ArQueryPageInformation( &srcSp->PageDirectory, src_addr, &paS, &atS );
	ArMapOnePage( &destSp->PageDirectory, dest_addr, paS, atS, flag );
	ArMapOnePage( &srcSp->PageDirectory, src_addr, paD, atD, flag );
	return 0;
}

int MmSwapMultiplePhysicalPages( KSpace* destSp, size_t dest_addr, KSpace* srcSp, size_t src_addr, size_t siz, uint flag )
{
	uint count;
	int result; 
	KVirtualMemoryAllocation* vma;
	if( dest_addr % PAGE_SIZE || src_addr % PAGE_SIZE || siz % PAGE_SIZE )
		return 0;
	vma = MmGetVmaByAddress( &srcSp->VirtualMemory, src_addr );
	if( !(vma->AllocationFlag & ALLOC_SWAP) || src_addr+siz > vma->VirtualAddress+vma->VirtualSize )
		return 0;
	vma = MmGetVmaByAddress( &destSp->VirtualMemory, dest_addr );
	if( !(vma->AllocationFlag & ALLOC_SWAP) || dest_addr+siz > vma->VirtualAddress+vma->VirtualSize )
		return 0;
	count = siz >> PAGE_SIZE_BITS;
	for( ; count; count--, dest_addr+=PAGE_SIZE, src_addr+=PAGE_SIZE )
		if( ( result = MmSwapPhysicalPage( destSp, dest_addr, srcSp, src_addr, flag ) ) < 0 )
			return siz-count*PAGE_SIZE;
	return siz;
}

int	MmDuplicatePhysicalPage( KSpace* destSp, size_t dest_addr, KSpace* srcSp, size_t src_addr )
{
	size_t paS, paD;
	uint atS, atD = 0;
	if( ArQueryPageInformation( &srcSp->PageDirectory, src_addr, &paS, &atS ) < 0 )
		return -ERR_INVALID;
	ArQueryPageInformation( &destSp->PageDirectory, dest_addr, &paD, &atD );
	if( atD&PAGE_ATTR_PRESENT )
		return -ERR_INVALID;
	IncreasePhysicalPageReference( paS );
	atS &= ~PAGE_ATTR_WRITE;
	ArMapOnePage( &destSp->PageDirectory, dest_addr, paS, atS, MAP_ADDRESS|MAP_ATTRIBUTE );
	ArMapOnePage( &srcSp->PageDirectory, src_addr, paS, atS, MAP_ADDRESS|MAP_ATTRIBUTE );
	return 0;
}

int 	MmDuplicateMultiplePhysicalPages( KSpace* destSp, size_t dest_addr, KSpace* srcSp, size_t src_addr, size_t siz )
{
	uint count;
	int result; 
	KVirtualMemoryAllocation* vma;
	if( dest_addr % PAGE_SIZE || src_addr % PAGE_SIZE || siz % PAGE_SIZE )
		return 0;
	vma = MmGetVmaByAddress( &srcSp->VirtualMemory, src_addr );
	if( !(vma->MemoryAttribute & PAGE_ATTR_WRITE) || src_addr+siz > vma->VirtualAddress+vma->VirtualSize )
		return 0;
	vma->AllocationFlag |= PAGE_ATTR_COPYONWRITE;
	vma = MmGetVmaByAddress( &destSp->VirtualMemory, dest_addr );
	if( !(vma->MemoryAttribute & PAGE_ATTR_WRITE) || dest_addr+siz > vma->VirtualAddress+vma->VirtualSize
		|| !(vma->AllocationFlag&ALLOC_VIRTUAL) )
		return 0;
	vma->AllocationFlag |= PAGE_ATTR_COPYONWRITE;
	count = siz >> PAGE_SIZE_BITS;
	for( ; count; count--, dest_addr+=PAGE_SIZE, src_addr+=PAGE_SIZE )
		if( ( result = MmDuplicatePhysicalPage( destSp, dest_addr, srcSp, src_addr ) ) < 0 )
			return siz-count*PAGE_SIZE;
	vma->AllocationFlag &= ~ALLOC_VIRTUAL;
	return siz;
}

int 	MmMapMemory( KSpace* space, uint virt_addr, uint phys_addr, uint size, uint attr, uint flag )
{
	KVirtualMemoryAllocation* vma;
	size_t q_phys=0, q_attr=0;
	if( IS_KERNEL_MEMORY( virt_addr ) )
		vma = MmGetVmaByAddress( &KernelVirtualMemory, virt_addr );
	else if( IS_USER_MEMORY(virt_addr) )
		vma = MmGetVmaByAddress( &space->VirtualMemory, virt_addr );
	else if( IS_GLOBAL_MEMORY(virt_addr) )
		vma = MmGetVmaByAddress( &GlobalVirtualMemory, virt_addr );
	if( !vma || !(vma->AllocationFlag&ALLOC_VIRTUAL) || virt_addr+size>vma->VirtualAddress+vma->VirtualSize ){
		return -ERR_INVALID;
	}
	ArQueryPageInformation( &space->PageDirectory, virt_addr, &q_phys, &q_attr );
	if( q_attr&PAGE_ATTR_PRESENT ){
		PERROR("The page map to has physaddr: %x or attr:%x", q_phys, q_attr);
		return -ERR_INVALID;
	}
	attr |= vma->MemoryAttribute;
	ArMapMultiplePages( &space->PageDirectory, virt_addr, phys_addr, size, attr, flag|MAP_ATTRIBUTE );
	return 0;
}

int	MmHandleReadOnlyPageFault( KSpace* space, size_t addr )
{
	KVirtualMemoryAllocation* vma;
	uint eflags;
	uint phys_addr, temp_addr, attr;
	if( IS_KERNEL_MEMORY( addr ) )
		vma = MmGetVmaByAddress( &KernelVirtualMemory, addr );
	else	
		vma = MmGetVmaByAddress( &space->VirtualMemory, addr );
	if( vma && vma->AllocationFlag & PAGE_ATTR_COPYONWRITE ){
//		PERROR("Copy On Write at 0x%x", addr );
		addr = (addr>>PAGE_SIZE_BITS)<<PAGE_SIZE_BITS;
		if( ArQueryPageInformation( &space->PageDirectory, addr, &phys_addr, &attr ) < 0 )
			return 0;
		if( GetPhysicalPageReference( phys_addr ) > 1 ){
			phys_addr= MmGetPhysicalPage();
			if( !phys_addr )
				KERROR("##no page for allocation.");
			ArLocalSaveIrq( eflags ); //spinlock too...
			temp_addr = ArMapTemporaryPage( phys_addr );
			RtlCopyMemory32( (void*)temp_addr, (void*)addr, PAGE_SIZE>>2 );
			ArLocalRestoreIrq( eflags );
			ArMapOnePage( &space->PageDirectory, addr, phys_addr, 
				vma->MemoryAttribute, MAP_ADDRESS|MAP_ATTRIBUTE );
			MmFreePhysicalPage( phys_addr );
		}else{
			ArMapOnePage( &space->PageDirectory, addr, 0, vma->MemoryAttribute, MAP_ATTRIBUTE );
		}
		return 1;
	}
	return 0;
}

int	MmHandleNonPresentPageFault( KSpace* space, size_t addr )
{
	KVirtualMemoryAllocation* vma;
	if( IS_KERNEL_MEMORY( addr ) )
		vma = MmGetVmaByAddress( &KernelVirtualMemory, addr );
	else if( IS_USER_MEMORY(addr) )
		vma = MmGetVmaByAddress( &space->VirtualMemory, addr );
	else if( IS_GLOBAL_MEMORY(addr) )
		vma = MmGetVmaByAddress( &GlobalVirtualMemory, addr );
	else
		KeBugCheck("MmHandleNonPresentPageFault()");
	if( !vma ){
		PERROR("Access violation at 0x%X", addr );
		return 0;
	}
	if( vma->AllocationFlag & ALLOC_LAZY  ){
		uint mapFlag = MAP_ATTRIBUTE;
		if( vma->AllocationFlag&ALLOC_ZERO )
			mapFlag |= MAP_ZERO;
		addr = (addr>>PAGE_SIZE_BITS)<<PAGE_SIZE_BITS;
		if( 0!= MmAcquirePhysicalPage( space, addr, vma->MemoryAttribute, mapFlag ) )
			KERROR("##no page for allocation.");
		return 1;
	}
	KERROR("Is this page swapped out? addr=0x%X", addr);
	return 0;
}
