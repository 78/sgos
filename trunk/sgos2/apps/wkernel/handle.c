#include <sgos.h>
#include <api.h>
#include "debug.h"
#include "kernel.h"


uint WINAPI GetStdHandle(uint type)
{
	switch( type ){
	case STD_INPUT_HANDLE:
		return type;
	case STD_OUTPUT_HANDLE:
		return type;
	case STD_ERROR_HANDLE:
		return type;
	default:
		DBG("Unknown handle type: %x", type );
	}
	return 0;
}


void WINAPI SetHandleInformation()
{
	NOT_IMPLEMENTED();
}


void WINAPI SetStdHandle()
{
	NOT_IMPLEMENTED();
}


void WINAPI CloseHandle()
{
	NOT_IMPLEMENTED();
}


void WINAPI DuplicateHandle()
{
	NOT_IMPLEMENTED();
}

