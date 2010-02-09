//map virtual memory to physical memory
//sgos2
//huang guan 090806

#include <sgos.h>
#include <arch.h>
#include <kd.h>
#include <mm.h>
#include <tm.h>
#include <rtl.h>

//映射一个临时页，虚拟地址是占用了一个页目录的地址
size_t ArMapTemporaryPage( size_t phys_addr )
{
	const int index = 0x00303000 >> 12;
	PAGE_TABLE* te = (PAGE_TABLE*)(KERNEL_BASE + 0x00301000) + index;
	te->v = phys_addr;
	te->a.present = te->a.write = 1;
	ArFlushPageTableEntry( KERNEL_BASE + 0x00303000 );
	return KERNEL_BASE + 0x00303000;
}

// get page information
int ArQueryPageInformation( uint dir, uint virt_addr, uint* phys_addr, uint *attr )
{
	PAGE_DIR* de, *te, *table, d;
	uint eflags;
	ASSERT( virt_addr%PAGE_SIZE == 0 );
	// get page directory entry
	de = (PAGE_DIR*)dir + (virt_addr>>22);
	if( !de->v )	//no table
		return -1;
	// disable interrupts when we use ArMapTemporaryPage(spinlock is a must too)
	ArLocalSaveIrq( eflags );
	// get page table entry
	table = (PAGE_DIR*)ArMapTemporaryPage(de->a.physicalAddress<<PAGE_SIZE_BITS); //SPACE_PAGETABLE_BEG + (virt_addr>>12);
	te = table + ((virt_addr<<10)>>22);
	d = *te;
	ArLocalRestoreIrq( eflags );
	*phys_addr = d.a.physicalAddress<<12;
	*attr = 0;
	if( d.a.user )
		*attr |= PAGE_ATTR_USER;
	if( d.a.write )
		*attr |= PAGE_ATTR_WRITE;
	if( d.a.present )
		*attr |= PAGE_ATTR_PRESENT;
	if( d.a.allocated )
		*attr |= PAGE_ATTR_ALLOCATED;
	if( d.a.copyOnWrite )
		*attr |= PAGE_ATTR_COPYONWRITE;
	if( d.a.share )
		*attr |= PAGE_ATTR_SHARE;
	return 0;
}

// map only one page from virtual address to physical address
void ArMapOnePage( uint dir, uint virt_addr, uint phys_addr, uint attr, uint flag )
{
	PAGE_DIR* de, *te, *table;
	uint eflags;
	int newpage=0;
	ASSERT( phys_addr%PAGE_SIZE == 0 &&
		virt_addr%PAGE_SIZE == 0 );
	// get page directory entry
	de = (PAGE_DIR*)dir + (virt_addr>>22);
	if( !de->v ){	//no table
		newpage = 1;
		de->v = ArGetPhysicalPage();
		if( !de->v )
			return;
		de->a.user = 1;
		de->a.write = 1;
		de->a.present = 1;
	}
	// disable interrupts when we use ArMapTemporaryPage(spinlock is a must too)
	ArLocalSaveIrq( eflags );
	// get page table entry
	table = (PAGE_DIR*)ArMapTemporaryPage(de->a.physicalAddress<<PAGE_SIZE_BITS); //SPACE_PAGETABLE_BEG + (virt_addr>>12);
	te = table + ((virt_addr<<10)>>22);
	if( newpage )
		RtlZeroMemory32( te, PAGE_SIZE>>2 );
	if( flag&MAP_ADDRESS && te->v && te->a.physicalAddress!=(phys_addr>>12) && te->a.write )
		PERROR("## Leaking memory at 0x%X", virt_addr );
	//设置新的值
	if( flag&MAP_ADDRESS )
		te->a.physicalAddress = phys_addr>>12;
	if( flag&MAP_ATTRIBUTE ){
		if( attr&PAGE_ATTR_USER )
			te->a.user = 1;
		if( attr&PAGE_ATTR_WRITE )
			te->a.write = 1;
		if( attr&PAGE_ATTR_PRESENT )
			te->a.present = 1;
		if( attr&PAGE_ATTR_ALLOCATED )
			te->a.allocated = 1;
		if( attr&PAGE_ATTR_COPYONWRITE )
			te->a.copyOnWrite = 1;
		if( attr&PAGE_ATTR_SHARE )
			te->a.share = 1;
	}
	ArLocalRestoreIrq( eflags );
	if( MmGetCurrentSpace()->PageDirectory == dir )
		//更改了分页信息，刷新页目录
		ArFlushPageTableEntry( virt_addr );
}

//取消一个页面的映射
void ArUnmapOnePage( uint dir, uint virt_addr )
{
	uint phys_addr, attr;
	if( ArQueryPageInformation( dir, virt_addr, &phys_addr, &attr ) == 0 ){
		//clear the table entry value!
		ArMapOnePage( dir, virt_addr, 0, 0, MAP_ADDRESS|MAP_ATTRIBUTE );
	}
}


// map virutal space to physical space
void ArMapMultiplePages( uint dir, uint virt_addr, uint phys_addr, uint size, uint attr, uint flag )
{
	uint count;
	count = size >> PAGE_SIZE_BITS;
//	KdPrintf("map vir:0x%X to phy:0x%X  size:0x%X\n", virt_addr, phys_addr, size );
	for( ; count; count--, virt_addr+=PAGE_SIZE, phys_addr+=PAGE_SIZE ){
		ArMapOnePage( dir, virt_addr, phys_addr, attr, flag );
	}
}

//取消一堆页面的映射。。。
void ArUnmapMultiplePages( uint dir, uint virt_addr, uint size )
{
	uint count;
	count = size >> PAGE_SIZE_BITS;
	for( ; count; count--, virt_addr+=PAGE_SIZE ){
		ArUnmapOnePage( dir, virt_addr );
	}
}
