#ifndef _MODULE_H
#define _MODULE_H

#include "pe.h"

#define MAX_IMPORT_MODULES 64

typedef struct ModuleInSpace{
	struct ModuleInSpace*	prev, *next;
	uint		SpaceId;
	size_t		VirtualAddress;	//Load address
}ModuleInSpace;

typedef struct PeModule{
	uint		ModuleId;
	char		Path[PATH_LEN];	//模块绝对路径
	ushort		Reference;
	size_t		LoadAddress;
	size_t		ImageSize;
	size_t		ImageBase;
	size_t		EntryAddress;
	size_t		StackLimit;
	IMAGE_OPTIONAL_HEADER OptionalHeader;
	PeModule**	ImportModules;
	int		ImportModuleCount;
	ModuleInSpace*	LoadedInformation;
}PeModule;

PeModule* AllocateModule( const char* path );
void FreeModule( PeModule* mo, uint spaceId );
size_t GetProcedureAddress( PeModule* mo, ModuleInSpace* mi, const char* procedureName );
int LinkModuleToSpace( PeModule* mo, uint spaceId );
ModuleInSpace* GetLoadedInformation( PeModule* mo, uint space );
PeModule* GetModuleById( uint id );
PeModule* GetModuleByPath( const char* path );
PeModule* GetModuleByName( const char* name );

int PeLoadLibrary( uint spaceId, const char * path );
int PeUnloadLibrary( uint spaceId, uint mid );
size_t PeGetProcedureAddress( uint spaceId, uint mid, const char* name );

#endif
