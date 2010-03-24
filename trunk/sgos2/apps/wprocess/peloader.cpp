#include <sgos.h>
#include <api.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "module.h"
#include "pe.h"

#define MIN(a,b) (a>b?b:a)

int PeLoadLibrary( uint spaceId, const char * path )
{
	FILEBUF* fb=0;
	size_t addr = 0;
	PeModule *mo = 0;
	FILEHDR coffhdr;
	IMAGE_OPTIONAL_HEADER opthdr;
	IMAGE_DOS_HEADER doshdr;
	SECHDR sechdr;
	size_t filesize, imgsize;
	int result, i, pos;
	// Check if it is loaded
	mo = GetModuleByPath( path );
	if( mo ){
		if( GetLoadedInformation( mo, spaceId ) ){
			return 0; //Already loaded into the requied space.
		}else{
			result = LinkModuleToSpace( mo, spaceId );
			return result;
		}
	}
	printf("[pe]Load %s\n", path );
	fb= FsOpenFile(path, FILE_READ, 0);
	if( !fb ){ 
		result = -ERR_WRONGARG; //Should be file not found
		goto bed;
	}
	filesize = fb->filesize;
	//Read the file
	FsReadFile( fb, (uchar*)&doshdr, sizeof(doshdr) );
	if( doshdr.e_magic != IMAGE_DOS_SIGNATURE ){
		printf("[pe]dos header not found.");
		result = -0x1001;
		goto bed;
	}
	FsSetFilePointer( fb, doshdr.e_lfanew+4, SEEK_SET );
	FsReadFile( fb, (uchar*)&coffhdr, sizeof(coffhdr) );
	if( !(coffhdr.usFlags&F_EXEC) || !coffhdr.usOptHdrSZ )
	{
		printf("[pe]not an executable file.");
		result = -0x1002;
		goto bed;
	}
	FsReadFile( fb, (uchar*)&opthdr, MIN(sizeof(opthdr),coffhdr.usOptHdrSZ) );
	if( opthdr.Magic != 0x10B ){
		printf("[pe]not a PE32file.");
		result = -0x1003;
		goto bed;
	}
	imgsize = PAGE_ALIGN(opthdr.SizeOfImage);
	addr = (size_t)SysAllocateMemory( SysGetCurrentSpaceId(), imgsize, MEMORY_ATTR_WRITE, ALLOC_SWAP ); //ALLOC_VIRTUAL
	if( addr == 0 ){
		result = -ERR_NOMEM;
		goto bed;
	}
	//read sections!
	pos = doshdr.e_lfanew+4+sizeof(coffhdr)+coffhdr.usOptHdrSZ;
	for( i=0; i<coffhdr.usNumSec; i++ ){
		FsSetFilePointer( fb, pos, SEEK_SET );
		if( FsReadFile( fb, (uchar*)&sechdr, sizeof(SECHDR) ) != sizeof(SECHDR) ){
			result = -ERR_IO;
			break;
		}
		size_t dest = sechdr.ulVAddr+addr;
		FsSetFilePointer( fb, sechdr.ulSecOffset, SEEK_SET );
//		printf("Load at %x:%x\n", dest, sechdr.ulSize );
		if( FsReadFile( fb, (uchar*)dest, sechdr.ulSize ) != sechdr.ulSize ){
			result = -ERR_IO;
			break;
		}
		pos += sizeof(SECHDR);
	}
	FsCloseFile( fb );
	fb = 0;
	mo = AllocateModule( path );
	if( !mo ){
		result = -ERR_NOMEM;
		goto bed;
	}
	mo->LoadAddress = addr;
	mo->ImageSize = imgsize;
	mo->ImageBase = opthdr.ImageBase;
	mo->StackLimit = opthdr.SizeOfStackReserve;
	mo->EntryAddress = mo->ImageBase + opthdr.AddressOfEntryPoint;
	printf("[pe]LoadAddress %x:%x:%x\n", addr, imgsize, mo->EntryAddress );
	mo->OptionalHeader = opthdr;
	result = LinkModuleToSpace( mo, spaceId );
	if( result < 0 ){
		FreeModule( mo, 0 );
		return result;
	}
	//Success :)
	return 0;
bed:
	printf("[pe]Failed in load : result=%d\n", result);
	if( fb )
		FsCloseFile(fb);
	if( addr )
		SysFreeMemory( SysGetCurrentSpaceId(), (void*)addr );
	if( mo )
		FreeModule( mo, spaceId );
	return result;
}

int PeUnloadLibrary( uint spaceId, uint mid )
{	
	PeModule* mo = GetModuleById( mid );
	if( !mo )
		return -ERR_WRONGARG;
	FreeModule( mo, spaceId );
	return 0;
}

size_t PeGetProcedureAddress( uint spaceId, uint mid, const char* name )
{
	PeModule* mo= GetModuleById( mid );
	if( !mo )
		return 0;
	ModuleInSpace* mi = GetLoadedInformation( mo, spaceId );
	if( !mi )
		return 0;
	return GetProcedureAddress( mo, mi, name );
}

