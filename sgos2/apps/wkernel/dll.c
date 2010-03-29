#include <sgos.h>
#include <api.h>
#include "debug.h"


void WINAPI FreeLibrary(int mid)
{
	return PsFreeModule( mid );
}


int WINAPI LoadLibraryA(const char* filename)
{
	return PsLoadModule( filename );
}


int WINAPI LoadLibraryExA(const char* filename, int fd, uint flag)
{
	DBG("Warning: Omitting flag %x", flag );
	return LoadLibraryA(filename);
}


size_t WINAPI GetModuleFileNameA( int mid, const char* filename, int siz )
{
	NOT_IMPLEMENTED();
	return 0;
}

//返回一个加载的模块的句柄
//if input NULL, return the module to the file used to create the calling process 
int WINAPI GetModuleHandleA( const char* mname){
	DBG("mname:%s", mname );
	if( !mname ){
		ProcessInformation* pi = GetCurrentProcessInformation();
		return pi->ModuleId;
	}
	int mid = PsGetModule( mname, NULL, 0 );
	return mid;
}

size_t WINAPI GetProcAddress( int mid, const char* name )
{
	DBG("mid:%x name:%s", mid, name );
	size_t addr = PsGetProcedure( mid, name );
	return addr;
}
