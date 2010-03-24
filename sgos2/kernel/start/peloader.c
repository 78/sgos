//Pe Loader
//By Xiaoxia

#include <sgos.h>
#include <ke.h>
#include <kd.h>
#include <tm.h>
#include <mm.h>
#include <rtl.h>
#include "pe.h"

// COpy Pe Segments
static int CopyPeSegments( KSpace *space, FILEHDR* coffhdr, SECHDR* sechdr, size_t base, size_t addr )
{
	//加载段数据
	int i;
	for( i=0; i<coffhdr->usNumSec; i++ ){
		size_t load_addr = sechdr[i].ulVAddr + base, virt_size = PAGE_ALIGN(sechdr[i].ulSize);
		if( virt_size == 0 )//bss size could be zero!
			virt_size = PAGE_SIZE;
		uint attr = PAGE_ATTR_WRITE;
		if( sechdr[i].ulFlags&STYP_TEXT) 
			attr &= ~PAGE_ATTR_WRITE;
		void* ptr = MmAllocateUserMemoryAddress(space, load_addr, virt_size, attr, ALLOC_ZERO|ALLOC_SWAP );
		void* ptrHere = MmAllocateUserMemory(MmGetCurrentSpace(), virt_size, PAGE_ATTR_WRITE, ALLOC_ZERO|ALLOC_SWAP );
		if( ptr == NULL ){
			PERROR("Failed to allocate memory at 0x%X. SpaceId:%X", load_addr, space->SpaceId );
			return -5;
		}
		if( sechdr[i].ulSize > 0 ){
			RtlCopyMemory( ptrHere, (void*)(addr+sechdr[i].ulSecOffset), sechdr[i].ulSize );
			if( MmSwapMultiplePhysicalPages(space, load_addr, MmGetCurrentSpace(), 
				(size_t)ptrHere, virt_size, MAP_ADDRESS ) != virt_size ){
				PERROR("Failed to write allocated mem at 0x%X.", load_addr );
				MmFreeUserMemory(MmGetCurrentSpace(), ptrHere);
				return -6;
			}
		}
		MmFreeUserMemory(MmGetCurrentSpace(), ptrHere);
	}
	return 0;
}

// 加载PE镜像文件
int KeLoadPeExecutable(KSpace* space, size_t addr, size_t siz, size_t *ret_addr)
{
	size_t entry, base, imageSize;
	IMAGE_DOS_HEADER* doshdr = (IMAGE_DOS_HEADER*)addr;
	if( doshdr->e_magic != IMAGE_DOS_SIGNATURE ){
		PERROR("dos header not found.");
		return -1;
	}
	FILEHDR* coffhdr = (FILEHDR*)(addr+doshdr->e_lfanew+4);
	if( !(coffhdr->usFlags&F_EXEC) || !coffhdr->usOptHdrSZ )
	{
		PERROR("not an executable file.");
		return -2;
	}
	IMAGE_OPTIONAL_HEADER* opthdr = (IMAGE_OPTIONAL_HEADER*)((size_t)coffhdr + sizeof(FILEHDR));
	if( opthdr->Magic != 0x10B ){
		PERROR("not a PE32file.");
		return -3;
	}
	entry = opthdr->AddressOfEntryPoint + opthdr->ImageBase;
	base = opthdr->ImageBase;
	imageSize = opthdr->SizeOfImage;
	SECHDR* sechdr = (SECHDR*)((size_t)opthdr + coffhdr->usOptHdrSZ);
	if( CopyPeSegments( space, coffhdr, sechdr, base, addr ) != 0 )
		return -7;
	*ret_addr = entry;
	return 0;
}


