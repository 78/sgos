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
		if( attr & PAGE_ATTR_ALLOCATED ){
			//clear the table entry value!
			ArMapOnePage( &space->PageDirectory, addr, 0, 0, MAP_ATTRIBUTE );
		}
		if( attr & PAGE_ATTR_PRESENT ){
			//free the page
			MmFreePhysicalPage( phys_addr );
		}
	}
}
/*
//由0xE0000000上的页目录虚拟地址得到物理页面的地址
uint ArVirtualToPhysicalAddress( uint virt_addr )
{
	PAGE_TABLE* te;
	te = (PAGE_TABLE*)SPACE_PAGETABLE_BEG + (virt_addr>>12);
	return ((te->a.physicalAddress)<<12);
}
*/

int MmSwapPhysicalPage( KSpace* destSp, size_t dest_addr, KSpace* srcSp, size_t src_addr, uint flag )
{
	size_t paS, paD;
	uint atS, atD;
	if( ArQueryPageInformation( &destSp->PageDirectory, dest_addr, &paD, &atD ) < 0 ||
		ArQueryPageInformation( &srcSp->PageDirectory, src_addr, &paS, &atS ) < 0 )
		return -ERR_INVALID;
	if( !(atD&PAGE_ATTR_ALLOCATED) || !(atS&PAGE_ATTR_ALLOCATED) )
		return -ERR_INVALID;
	ArMapOnePage( &destSp->PageDirectory, dest_addr, paS, atS, flag );
	ArMapOnePage( &srcSp->PageDirectory, src_addr, paD, atD, flag );
	return 0;
}

int MmSwapMultiplePhysicalPages( KSpace* destSp, size_t dest_addr, KSpace* srcSp, size_t src_addr, size_t siz, uint flag )
{
	uint count;
	int result; 
	if( dest_addr % PAGE_SIZE || src_addr % PAGE_SIZE || siz % PAGE_SIZE )
		return 0;
	count = siz >> PAGE_SIZE_BITS;
	for( ; count; count--, dest_addr+=PAGE_SIZE, src_addr+=PAGE_SIZE )
		if( ( result = MmSwapPhysicalPage( destSp, dest_addr, srcSp, src_addr, flag ) ) < 0 )
			return siz-count*PAGE_SIZE;
	return siz;
}
