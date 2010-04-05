#include <sgos.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <api.h>
#include "module.h"
#include "wprocess.h"
#include "pe.h"

#define 	MAX_MODULE	128
PeModule* 	moduleList[MAX_MODULE];
static int	moduleCount = 0;
static int 	moduleId = 0;

PeModule* GetModuleByPath( const char* path )
{
	for( int i=0; i<moduleCount; i++ )
		if( strnicmp( (char*)path, moduleList[i]->Path, PATH_LEN ) == 0 ){
			return moduleList[i];
		}
	return 0;
}

PeModule* GetModuleByName( const char* name )
{
	const char *p, *q;
	p = strrchr( name, '/' );
	if( !p ) p = name;
	for( int i=0; i<moduleCount; i++ ){
		q = strrchr( moduleList[i]->Path, '/' );
		if( !q ) q = moduleList[i]->Path;
		if( strnicmp( (char*)p, (char*)q, PATH_LEN ) == 0 ){
			printf("[pe]Got Module %s\n", name );
			return moduleList[i];
		}
	}
	return 0;
}

PeModule* GetModuleById( uint id )
{
	for( int i=0; i<moduleCount; i++ )
		if( moduleList[i]->ModuleId == id ){
			return moduleList[i];
		}
	return 0;
}

ModuleInSpace* GetLoadedInformation( PeModule* mo, uint spaceId )
{
	for(ModuleInSpace* mi=mo->LoadedInformation; mi; mi=mi->next )
		if( mi->SpaceId == spaceId )
			return mi;
	return 0;
}

PeModule* AllocateModule( const char* path )
{
	PeModule* mo;
	mo = GetModuleByPath(path);
	if( mo )
		return mo;
	if( moduleCount >= MAX_MODULE )
		return 0;
	moduleList[moduleCount] = (PeModule*) malloc( sizeof(PeModule) );
	mo = moduleList[moduleCount++];
	memset( mo, 0, sizeof(*mo));
	strncpy( mo->Path, path, PATH_LEN-1 );
	mo->ModuleId = moduleId;
	moduleId ++;
	mo->ImportModuleCount = MAX_IMPORT_MODULES;
	mo->ImportModules = (PeModule**)malloc(sizeof(PeModule*)*mo->ImportModuleCount );
	return mo;
}

void FreeModule( PeModule* mo, uint spaceId )
{
	printf("[pe]FreeModule %s\n", mo->Path );
	if( spaceId ){
		ModuleInSpace *mi = GetLoadedInformation( mo, spaceId );
		if( mi ){
			if( mi->prev )
				mi->prev->next = mi->next;
			else
				mo->LoadedInformation = mi->next;
			if( mi->next )
				mi->next->prev = mi->prev;
			if( mi->VirtualAddress )
				SysFreeMemory( mi->SpaceId, (void*)mi->VirtualAddress );
		}else{
			//This space didn't own this module.
			return ;
		}
	}
	if( -- mo->Reference == 0 ){
		if( mo->LoadAddress )
			SysFreeMemory( SysGetCurrentSpaceId(), (void*)mo->LoadAddress );
		for( int i=0; i<moduleCount; i++ )
			if( moduleList[i] == mo ){
				if( i!=moduleCount-1 )
					moduleList[i] = moduleList[moduleCount-1];
				-- moduleCount;
				break;
			}
		free( mo->ImportModules );
		free( mo );
	}
}

size_t GetProcedureAddress( PeModule* mo, ModuleInSpace* mi, const char* procedureName )
{
	IMAGE_OPTIONAL_HEADER* opthdr = &mo->OptionalHeader;
	size_t addr = mo->LoadAddress;
	if( opthdr->NumberOfRvaAndSizes>0 && opthdr->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size>0 )
	{
		EXPORT_DIRECTORY_TABLE* exp =  (EXPORT_DIRECTORY_TABLE*)
			(opthdr->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].RVA+addr);
		char** ExpName = (char**)(addr + exp->NamePointerTable);
		t_16* ExpOrdinal = (t_16*)(addr + exp->OrdinalTable);
		size_t* ExpAddress = (size_t*)(addr + exp->ExportAddressTable);
		for( int i=0; i<exp->NumberOfNamePointers; i++ )
		{
			if( strcmp( (ExpName[i]+addr), procedureName ) == 0 ){
				return mi->VirtualAddress + ExpAddress[ExpOrdinal[i]];
			}
		}
	}
	printf("[pe]Failed to get address of %s in %s\n", procedureName, mo->Path );
	return 0;
}


// == LinkModuleToSpace ==
// Steps:
// * Duplicate Memory
// * Load Import Modules
// * Fixup Import Addresses
// * Fixup Relacations
// * Swap Memory
int LinkModuleToSpace( PeModule* mo, uint spaceId )
{
	int result;
	size_t addr = mo->LoadAddress;
	IMAGE_OPTIONAL_HEADER* opthdr = &mo->OptionalHeader;
	if( GetLoadedInformation( mo, spaceId ) )
		return 0;
	printf("[pe%d]Link %s for %x\n", SysGetCurrentSpaceId(), mo->Path, spaceId );
	ModuleInSpace* mi = (ModuleInSpace*)malloc(sizeof(ModuleInSpace) );
	if( !mi ){
		result = -ERR_NOMEM;
		goto bed;
	}
// * Duplicate Memory
	mi->VirtualAddress = (size_t)SysAllocateMemoryAddress( spaceId, mo->ImageBase, mo->ImageSize, 
		MEMORY_ATTR_WRITE, ALLOC_VIRTUAL|ALLOC_SWAP );
	if( mi->VirtualAddress == 0 ){
		printf("[pe] Failed to load module at 0x%x relocations is not supported yet!\n", mo->ImageBase );
		result = -ERR_NOIMP;
		goto bed;
	}
	if( SysDuplicateMemory( spaceId, mi->VirtualAddress, mo->LoadAddress, mo->ImageSize )!=mo->ImageSize ){
		printf("[pe] Failed while duplicating memory!\n");
		result -ERR_UNKNOWN;
		goto bed;
	}
// * Load Import Modules
// * Fixup Import Addresses
	if( opthdr->NumberOfRvaAndSizes>0 ){
		if( mo->ImportModuleCount > 0 ){
			IMPORT_DIRECTORY_TABLE* imp=(IMPORT_DIRECTORY_TABLE*)
				(addr + opthdr->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].RVA);
			for( int i=0; imp[i].Name && i<MAX_IMPORT_MODULES; i ++ ){
				uint mo2Id;
				const char* libName = (char*)(addr + imp[i].Name);
				mo2Id = PeLoadLibrary( spaceId, libName );
				if( mo2Id < 0 ){
					result = -ERR_UNKNOWN;
					goto bed;
				}
				PeModule* mo2 = GetModuleById( mo2Id );
				ModuleInSpace* mi2 = GetLoadedInformation( mo2, spaceId );
				mo->ImportModules[i] = mo2;
				for( 
					size_t	* lookupEntry = (size_t*)(imp[i].ImportLookupTable + addr ),
						* addressEntry = (size_t*)(imp[i].ImportAddressTable + addr );
					*lookupEntry; 
					lookupEntry++, addressEntry++ ){
					if( *lookupEntry & 0x80000000 ){
						printf("[pe]Import by ordinal is not implemented yet.\n");
						result = -ERR_NOIMP;
					}else{
						HINT_NAME_TABLE* hint = (HINT_NAME_TABLE*)
							( addr + (*lookupEntry & 0x7FFFFFFF) );
						size_t fixup = GetProcedureAddress( mo2, mi2, (char*)hint->Name );
						if( fixup ){
							*addressEntry = fixup;
						}else{
							*addressEntry = 0x66666666;
							result = -66;
							goto bed;
						}
					}
				}
			}
		}
	}
// * Fixup Relacations ( Not implemented. )

// * Swap Memory
	result = SysSwapMemory( spaceId, mi->VirtualAddress, addr, mo->ImageSize, MAP_ADDRESS );
	if( result != mo->ImageSize )
		printf("[pe]##Not all the memory are swapped result=0x%x.\n", result);
// * Add mi to mo
	mi->SpaceId = spaceId;
	mi->prev = 0;
	mi->next = mo->LoadedInformation;
	mo->LoadedInformation = mi;
	return 0;
bed:
	printf("[pe]LinkModuleToSpace result = %d\n", result );
	if( mi ){
		if( mi->VirtualAddress )
			SysFreeMemory( spaceId, (void*)mi->VirtualAddress );
		free( mi );
	}
	
	return result;
}
