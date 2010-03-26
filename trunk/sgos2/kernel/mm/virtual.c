// Bugfix : 2010 03 25
// Allocating a vma may cause a nonpresent page fault.
// The kernel would check the vma then we don't need to 
// acquire for teh access lock cuz we already have it.
// Better solution: Allocate the vma memory at first so 
// pagefault won't happen during the allocation.

#include <sgos.h>
#include <kd.h>
#include <mm.h>
#include <ipc.h>


void MmInitializeVirtualMemory( KVirtualMemory *vm, size_t beg, size_t end )
{
	vm->LowerLimit = beg;
	vm->UpperLimit = end;
	vm->FirstAllocation =
	vm->LastAllocation = NULL;
	IpcInitializeSemaphore( &vm->AccessLock );
}

static KVirtualMemoryAllocation* InsertPreviousVma( KVirtualMemory* vm, KVirtualMemoryAllocation* vma, KVirtualMemoryAllocation* next )
{
	vma->next = next;
	if( next ){
		if( next->prev ){
			next->prev->next = vma;
		}else{
			vm->FirstAllocation = vma;
		}
		vma->prev = next->prev;
		next->prev = vma;
	}else{
		vma->prev = vm->LastAllocation;
		if( vm->LastAllocation ){
			vm->LastAllocation->next = vma;
		}else{
			vm->FirstAllocation = vma;
		}
		vm->LastAllocation = vma;
	}
	return vma;
}

void DumpVirtualMemory( KVirtualMemory* vm )
{
	KVirtualMemoryAllocation* vma;
	KdPrintf("Dumping Virtual Memory \n");
	for( vma=vm->FirstAllocation; vma; vma=vma->next )
		KdPrintf("[%x-%x]", vma->VirtualAddress, vma->VirtualAddress+vma->VirtualSize );
	KdPrintf("\n");
}

void* MmAllocateVirtualMemory( KVirtualMemory* vm, size_t addr, size_t size, uint attr, uint flag )
{
	if( size < PAGE_SIZE || size%PAGE_SIZE != 0 )
		return NULL;
	KVirtualMemoryAllocation * vma=NULL,
		* vmabuf = (KVirtualMemoryAllocation*) MmAllocateKernelMemory( sizeof(KVirtualMemoryAllocation) );
	if( vmabuf == NULL )
		return NULL;
	down( &vm->AccessLock );
	if( flag & ALLOC_RANDOM ){
		if( flag & ALLOC_HIGHMEM ){
			addr = vm->UpperLimit - size;
			for( vma=vm->LastAllocation; vma; vma=vma->prev ){
				if( vma->VirtualAddress+vma->VirtualSize <= addr ){
					vma = InsertPreviousVma( vm, vmabuf, vma->next );
					goto GOOD;
				}
				addr = vma->VirtualAddress - size;
			}
			if( addr < vm->LowerLimit )
				goto BAD;
			vma = vmabuf;
			vma->next = vm->FirstAllocation;
			if( vm->FirstAllocation )
				vm->FirstAllocation->prev = vma;
			vm->FirstAllocation = vma;
		}else{ //ALLOC_LOWMEM
			addr = vm->LowerLimit;
			for( vma=vm->FirstAllocation; vma; vma=vma->next ){
				if( vma->VirtualAddress >= addr + size ){
					vma = InsertPreviousVma( vm, vmabuf, vma );
					goto GOOD;
				}
				addr = vma->VirtualAddress + vma->VirtualSize;
			}
			if( addr + size > vm->UpperLimit )
				goto BAD;
			vma = InsertPreviousVma( vm, vmabuf, NULL );
		}
	}else{
		if( addr < vm->LowerLimit || addr+size > vm->UpperLimit )
			goto BAD;
		for( vma=vm->FirstAllocation; vma && vma->VirtualAddress < addr; vma=vma->next )
			;
		if( vma && addr+size > vma->VirtualAddress )
			goto BAD;
		vma = InsertPreviousVma( vm, vmabuf, vma );
	}
GOOD:
	if( vma == NULL )
		goto BAD;
	vma->VirtualAddress = addr;
	vma->VirtualSize = size;
	vma->MemoryAttribute = attr;
	vma->AllocationFlag = flag;
	up( &vm->AccessLock );
	return (void*)addr;
BAD:
	up( &vm->AccessLock );
	MmFreeKernelMemory( vmabuf );
	return NULL;
}

int MmQueryVirtualAddressInformation( KVirtualMemory* vm, size_t addr, size_t* beg, size_t* end, uint *attr, uint *flag )
{
	KVirtualMemoryAllocation* vma;
	vma = MmGetVmaByAddress( vm, addr );
	if( !vma )
		return -ERR_NONE;
	*beg = vma->VirtualAddress;
	*end = vma->VirtualAddress + vma->VirtualSize;
	*attr = vma->MemoryAttribute;
	*flag = vma->AllocationFlag;
	return 0;
}

KVirtualMemoryAllocation* MmGetVmaByAddress( KVirtualMemory* vm, size_t addr )
{
	KVirtualMemoryAllocation* vma;
	down( &vm->AccessLock );
	for( vma=vm->LastAllocation; vma; vma=vma->prev )
		if( addr >= vma->VirtualAddress && addr < vma->VirtualAddress+vma->VirtualSize )
			break;
	up( &vm->AccessLock );
	return vma;
}

size_t MmFreeVirtualMemory( KVirtualMemory* vm, void* p )
{
	size_t siz = 0;
	KVirtualMemoryAllocation* vma = MmGetVmaByAddress( vm, (size_t) p );
	if( vma ){
		down( &vm->AccessLock );
		if( vma->prev )
			vma->prev->next = vma->next;
		else
			vm->FirstAllocation = vma->next;
		if( vma->next )
			vma->next->prev = vma->prev;
		else
			vm->LastAllocation = vma->prev;
		up( &vm->AccessLock );
		siz = vma->VirtualSize;
		MmFreeKernelMemory( vma );
		return siz;
	}
	PERROR("## Not found vma for 0x%X\n", p );
	return 0;
}
