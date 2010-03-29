#include <sgos.h>
#include <stdlib.h>
#include <api.h>
#include "debug.h"

#define LOCAL_STORAGE_SIZE 64
typedef struct LocalStorageUnit{
	uchar	used;
	void*	value;
}LocalStorageUnit;

uint WINAPI TlsAlloc()
{
	int i;
	ThreadInformation* ti= GetCurrentThreadInformation();
	if( ti->LocalStorage == NULL ){
		ti->LocalStorage = malloc( sizeof(LocalStorageUnit) * LOCAL_STORAGE_SIZE );
		memset( ti->LocalStorage, 0, sizeof(LocalStorageUnit) * LOCAL_STORAGE_SIZE );
		if( ti->LocalStorage == NULL )
			return 0xFFFFFFFF;
	}
	for( i=0; i<LOCAL_STORAGE_SIZE; i++ )
		if( !((LocalStorageUnit*)ti->LocalStorage)[i].used ){
			((LocalStorageUnit*)ti->LocalStorage)[i].used = 1;
			return i;
		}
	return 0xFFFFFFFF;
}


void WINAPI TlsFree(uint i)
{
	ThreadInformation* ti= GetCurrentThreadInformation();
	((LocalStorageUnit*)ti->LocalStorage)[i].used = 0;
}


void* WINAPI TlsGetValue(uint i)
{
	ThreadInformation* ti= GetCurrentThreadInformation();
	return ((LocalStorageUnit*)ti->LocalStorage)[i].value;
}


void WINAPI TlsSetValue(uint i, void* v)
{
	ThreadInformation* ti= GetCurrentThreadInformation();
	((LocalStorageUnit*)ti->LocalStorage)[i].value = v;
}
