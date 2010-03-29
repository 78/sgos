#include <sgos.h>
#include <api.h>
#include "debug.h"
#include "kernel.h"

void WINAPI CreateProcessA()
{
	NOT_IMPLEMENTED();
}


void WINAPI ExitProcess()
{
	NOT_IMPLEMENTED();
}


void WINAPI TerminateProcess()
{
	NOT_IMPLEMENTED();
}


void WINAPI OpenProcess()
{
	NOT_IMPLEMENTED();
}


uint WINAPI GetCurrentProcessId()
{
	ProcessInformation* pi = GetCurrentProcessInformation();
	if( pi && pi->ProcessId )
		return pi->ProcessId;
	return 0;
}


uint WINAPI GetCurrentProcess()
{
	uint pid = GetCurrentProcessId();
	if( pid )
		return _CreateHandle( TYPE_PROCESS, pid );
	return 0;
}


void WINAPI WriteProcessMemory()
{
	NOT_IMPLEMENTED();
}


void WINAPI GetProcessTimes()
{
	NOT_IMPLEMENTED();
}


void WINAPI GetExitCodeProcess()
{
	NOT_IMPLEMENTED();
}


void WINAPI ReadProcessMemory()
{
	NOT_IMPLEMENTED();
}


void WINAPI GetPriorityClass()
{
	NOT_IMPLEMENTED();
}


void WINAPI SetPriorityClass()
{
	NOT_IMPLEMENTED();
}


void WINAPI GetCurrentDirectoryA()
{
	NOT_IMPLEMENTED();
}


void WINAPI SetCurrentDirectoryA()
{
	NOT_IMPLEMENTED();
}
