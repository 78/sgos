#include <sgos.h>
#include <ke.h>
#include <kd.h>
#include <tm.h>
#include <mm.h>

static SystemInformation* systemInfo=NULL, *globalPage=NULL; //system inforamtion
static ServiceInformation * serviceList=NULL; //service list

void KeInitializeSystemInformation()
{
	size_t phys_addr, attr;
	systemInfo = (SystemInformation*)MmAllocateUserMemory( MmGetCurrentSpace(), PAGE_SIZE, MEMORY_ATTR_WRITE, 0 );
	if( systemInfo == NULL )
		KeBugCheck("Failed to allocate memory for SystemInformation.");
		
	RtlZeroMemory( systemInfo, PAGE_SIZE );
	systemInfo->PageSize = PAGE_SIZE;
	systemInfo->SystemVersion = (2<<16) | 1;
	systemInfo->TimeSlice = 1000 / RTC_FREQUENCY;
	serviceList = (ServiceInformation*)MmAllocateUserMemory( MmGetCurrentSpace(), SM_INFORMATION_SIZE, MEMORY_ATTR_WRITE, 0 );
	if( serviceList == NULL )
		KeBugCheck("Failed to allocate memory for ServiceList.");
	RtlZeroMemory( serviceList, SM_INFORMATION_SIZE );
		
		
	systemInfo->ServiceList = (ServiceInformation*)MmAllocateGlobalMemory( SM_INFORMATION_SIZE, 0, ALLOC_VIRTUAL );
	if( systemInfo->ServiceList==NULL )
		KERROR("Failed to allocate global memory");
	if( ArQueryPageInformation( &MmGetCurrentSpace()->PageDirectory, (size_t)serviceList, (size_t*)&phys_addr, &attr ) <0 )
		KERROR("Failed to get physystemInfocal page");
	if( MmMapMemory( MmGetCurrentSpace(), (size_t)systemInfo->ServiceList, phys_addr, SM_INFORMATION_SIZE, 0, MAP_ADDRESS ) < 0 )
		KERROR("Failed to  map page");
		
		
	globalPage = (SystemInformation*)MmAllocateGlobalMemory( PAGE_SIZE, 0, ALLOC_VIRTUAL );
	if( globalPage==NULL )
		KERROR("Failed to allocate global memory");
	if( ArQueryPageInformation( &MmGetCurrentSpace()->PageDirectory, (size_t)systemInfo, (size_t*)&phys_addr, &attr ) <0 )
		KERROR("Failed to get physystemInfocal page");
	if( MmMapMemory( MmGetCurrentSpace(), (size_t)globalPage, phys_addr, PAGE_SIZE, 0, MAP_ADDRESS ) < 0 )
		KERROR("Failed to  map page");
	KdPrintf("System Information Page at 0x%X, version: %x\n", systemInfo, globalPage->SystemVersion );
	return;
}

SystemInformation* KeGetSystemInforamtion()
{
	if( !globalPage )
		KeBugCheck("Bug: SystemInformation is not initialized yet.");
	return globalPage;
}
