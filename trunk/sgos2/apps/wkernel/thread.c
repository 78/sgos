#include <sgos.h>
#include <api.h>
#include "debug.h"
#include "kernel.h"

void WINAPI ExitThread()
{
	NOT_IMPLEMENTED();
}


void WINAPI CreateThread()
{
	NOT_IMPLEMENTED();
}


void WINAPI SetThreadContext()
{
	NOT_IMPLEMENTED();
}


void WINAPI SetThreadPriority()
{
	NOT_IMPLEMENTED();
}


void WINAPI TerminateThread()
{
	NOT_IMPLEMENTED();
}


void WINAPI ResumeThread()
{
	NOT_IMPLEMENTED();
}


void WINAPI SuspendThread()
{
	NOT_IMPLEMENTED();
}


void WINAPI GetThreadContext()
{
	NOT_IMPLEMENTED();
}


void WINAPI GetStartupInfoA(STARTUPINFO* startupinfo)
{
	startupinfo->hStdInput = STD_INPUT_HANDLE;
	startupinfo->hStdOutput = STD_OUTPUT_HANDLE;
	startupinfo->hStdError = STD_ERROR_HANDLE;
}


uint WINAPI GetCurrentThreadId()
{
	ThreadInformation* ti = GetCurrentThreadInformation();
	if( ti && ti->ThreadId)
		return ti->ThreadId;
	return SysGetCurrentThreadId();
}

uint WINAPI GetCurrentThread()
{
	uint tid = GetCurrentThreadId();
	if( tid )
		return _CreateHandle( TYPE_THREAD, tid );
	return 0;
}

