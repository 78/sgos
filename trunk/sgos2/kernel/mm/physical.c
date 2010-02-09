//physical memory

#include <sgos.h>
#include <kd.h>
#include <mm.h>

// 为虚拟地址申请多个物理页面
int	MmAcquireMultiplePhysicalPages(uint dir, size_t addr, size_t siz, uint attr, uint flag)
{
	uint count;
	count = siz >> PAGE_SIZE_BITS;
	for( ; count; count--, addr+=PAGE_SIZE ){
		if( MmAcquirePhysicalPage( dir, addr, attr, flag )!=0 )
			return siz-count*PAGE_SIZE;
	}
	return siz;
}

// 为虚拟地址申请多个物理页面
void	MmReleaseMultiplePhysicalPages(uint dir, size_t addr, size_t siz)
{
	uint count;
	count = siz >> PAGE_SIZE_BITS;
	for( ; count; count--, addr+=PAGE_SIZE )
		MmReleasePhysicalPage( dir, addr );
}

// 为虚拟地址申请一个物理页面
int	MmAcquirePhysicalPage(uint dir, size_t addr, uint attr, uint flag)
{
	// 分配物理页面
	uint phys_addr = ArGetPhysicalPage();
	if( !phys_addr )
		return -ERR_NOMEM;
	//映射到分配的物理地址去。
	ArMapOnePage( dir, addr, phys_addr, attr|PAGE_ATTR_PRESENT, MAP_ADDRESS|flag );
	return 0;
}

// 释放虚拟地址上的物理页面
void	MmReleasePhysicalPage(uint dir, size_t addr)
{
	uint phys_addr, attr;
	if( ArQueryPageInformation( dir, addr, &phys_addr, &attr ) == 0 ){
		//clear the table entry value!
		ArMapOnePage( dir, addr, 0, 0, MAP_ADDRESS|MAP_ATTRIBUTE );
		//free the page
		ArFreePhysicalPage( phys_addr );
	}
}

