#include <sgos.h>
#include <api.h>
#include "debug.h"
#include "kernel.h"

#define MAX_HANDLES_PER_PROCESS 1024
#define HANDLE_MAGIC 0xFBFC1314

typedef struct HandleInformationSet{
	uint			magic;
	struct HandleInformationSet* prev, *next;
	int 			type;
	uint			data;
	uint			mask;
	uint			flag;
}HandleInformation;


uint _CreateHandle( int type, uint data )
{
	ProcessInformation* pi = GetCurrentProcessInformation();
	if( pi ){
		HandleInformation* hi = (HandleInformation*)malloc( sizeof(HandleInformation) );
		if( hi==NULL )
			return 0;
		hi->type = type;
		hi->data = data;
		hi->magic = HANDLE_MAGIC;
		hi->mask = hi->flag = 0;
		//## Lock here
		hi->next = pi->HandleSet;
		hi->prev = 0;
		if( pi->HandleSet )
			((HandleInformation*)pi->HandleSet)->prev = hi;
		pi->HandleSet = hi;
		//## Unlock
		return (uint)hi;
	}
	return 0;
}

uint _GetHandleType(uint h )
{
	switch( h ){
	case STD_INPUT_HANDLE:
	case STD_OUTPUT_HANDLE:
	case STD_ERROR_HANDLE:
		return TYPE_CHAR;
	}
	HandleInformation* hi = (HandleInformation*)h;
	if( !hi || hi->magic != HANDLE_MAGIC )
		return TYPE_UNKNOWN;
	return hi->type;
}

void _CloseHandle( uint h )
{
	ProcessInformation* pi = GetCurrentProcessInformation();
	HandleInformation* hi = (HandleInformation*)h;
	if( !hi || hi->magic != HANDLE_MAGIC )
		return ;
	switch( hi->type ){
	case TYPE_FILE:
		FsCloseFile( (void*)hi->data );
		break;
	case TYPE_CHAR:
		break;
	default:
		DBG("Unknown handle type.");
	}
	//## Lock here
	if( hi->next )
		hi->next->prev = hi->prev;
	if( hi->prev )
		hi->prev->next = hi->next;
	else
		pi->HandleSet = hi->next;
	//## Unlock here
}

uint _GetHandleData( uint h )
{
	HandleInformation* hi = (HandleInformation*)h;
	if( !hi || hi->magic != HANDLE_MAGIC )
		return ;
	return hi->data;
}

