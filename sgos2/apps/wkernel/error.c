#include <sgos.h>
#include <api.h>
#include "debug.h"


void WINAPI SetLastError(uint code)
{
	ThreadInformation* ti = GetCurrentThreadInformation();
	if( ti )
		ti->ErrorCode = code;
}


uint WINAPI GetLastError()
{
	ThreadInformation* ti = GetCurrentThreadInformation();
	if( ti )
		return ti->ErrorCode;
	return -ERR_UNKNOWN;
}


void WINAPI SetErrorMode()
{
	NOT_IMPLEMENTED();
}
