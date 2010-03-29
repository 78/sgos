#include <sgos.h>
#include <api.h>
#include "debug.h"

#define MEM_LARGE_PAGES 0x20000000
#define MEM_PHYSICAL 0x400000
#define MEM_TOP_DOWN 0x100000
#define MEM_WRITE_WATCH 0x200000

typedef struct _MEMORY_BASIC_INFORMATION {
  void*  BaseAddress;
  void*  AllocationBase;
  uint  AllocationProtect;
  size_t RegionSize;
  uint  State;
  uint  Protect;
  uint  Type;
} MEMORY_BASIC_INFORMATION, *PMEMORY_BASIC_INFORMATION;

size_t WINAPI VirtualAlloc( void* addr, size_t siz, uint type, uint protect )
{
	uint flag=0;
	if( protect&MEM_TOP_DOWN )
		flag |= ALLOC_HIGHMEM;
	if( protect&MEM_PHYSICAL )
		flag |= ALLOC_VIRTUAL;
	if( addr == NULL )
		return (size_t)SysAllocateMemory( SysGetCurrentSpaceId(), siz, MEMORY_ATTR_WRITE, flag );
	else
		return (size_t)SysAllocateMemoryAddress( SysGetCurrentSpaceId(), (size_t)addr, siz, MEMORY_ATTR_WRITE, flag );
}

size_t WINAPI LocalAlloc( uint flag, size_t siz )
{
	return VirtualAlloc( NULL, siz, 0, 0 );
}

void WINAPI VirtualFree( const void* p, size_t siz, uint type )
{
	SysFreeMemory( SysGetCurrentSpaceId(), (void*)p );
}

size_t WINAPI VirtualQuery( const void* addr, MEMORY_BASIC_INFORMATION* info , size_t len )
{
	size_t beg, end;
	uint attr, flag;
	if( SysQueryAddress( SysGetCurrentSpaceId(), (size_t)addr, &beg, &end, &attr, &flag ) < 0 )
		return 0;
	if( sizeof(MEMORY_BASIC_INFORMATION)>len )
		return 0;
	info->BaseAddress = (void*)addr;
	info->AllocationBase = (void*)beg;
	info->AllocationProtect = 0;
	info->RegionSize = end-beg;
	info->State = 0;
	info->Type = 0;
	return sizeof(MEMORY_BASIC_INFORMATION);
}

