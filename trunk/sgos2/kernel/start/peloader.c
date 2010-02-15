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
		void* ptr = MmAllocateUserMemoryAddress(space, load_addr, virt_size, attr, ALLOC_ZERO );
		if( ptr == NULL ){
			PERROR("Failed to allocate memory at 0x%X. SpaceId:%X", load_addr, space->SpaceId );
			return -5;
		}
		if( sechdr[i].ulSize > 0 ){
			if( MmWriteUserMemory(space, load_addr, (void*)(addr+sechdr[i].ulSecOffset), 
				sechdr[i].ulSize ) != sechdr[i].ulSize ){
				PERROR("Failed to write allocated mem at 0x%X.", load_addr );
				return -6;
			}
			if( sechdr[i].ulFlags&STYP_TEXT) {
				attr &= ~PAGE_ATTR_WRITE;
				MmSetUserMemoryAttribute( space, load_addr, virt_size, PAGE_ATTR_PRESENT|attr );
			}
		}
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


