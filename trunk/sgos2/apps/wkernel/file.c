#include <sgos.h>
#include <api.h>
#include "debug.h"
#include "kernel.h"

void WINAPI CopyFileA()
{
	NOT_IMPLEMENTED();
}


void WINAPI CreateFileA()
{
	NOT_IMPLEMENTED();
}


void WINAPI DeleteFileA()
{
	NOT_IMPLEMENTED();
}


void WINAPI UnlockFile()
{
	NOT_IMPLEMENTED();
}


void WINAPI UnlockFileEx()
{
	NOT_IMPLEMENTED();
}


int WINAPI WriteFile(uint h, const void*p, uint c, uint* w,  void* overlapped)
{
	int ret;
	switch( h ){
	case STD_INPUT_HANDLE:
		NOT_IMPLEMENTED();
		return;
	case STD_OUTPUT_HANDLE:
		Api_Print( p, c );
		*w = c;
		return 1;
	case STD_ERROR_HANDLE:
		Api_Print( p, c );
		*w = c;
		return 1;
	}
	FILEBUF* fb = (FILEBUF*)_GetHandleData( h );
	ret = FsWriteFile( fb, (void*)p, c );
	*w = ret;
	return (ret>0);
}


void WINAPI SetFileApisToANSI()
{
	NOT_IMPLEMENTED();
}


void WINAPI SetFileApisToOEM()
{
	NOT_IMPLEMENTED();
}


void WINAPI SetFileAttributesA()
{
	NOT_IMPLEMENTED();
}


void WINAPI SetFilePointer()
{
	NOT_IMPLEMENTED();
}


void WINAPI SetEndOfFile()
{
	NOT_IMPLEMENTED();
}


void WINAPI LockFile()
{
	NOT_IMPLEMENTED();
}


void WINAPI LockFileEx()
{
	NOT_IMPLEMENTED();
}


void WINAPI MoveFileA()
{
	NOT_IMPLEMENTED();
}


void WINAPI MoveFileExA()
{
	NOT_IMPLEMENTED();
}


void WINAPI GetFileSize()
{
	NOT_IMPLEMENTED();
}


void WINAPI GetFileTime()
{
	NOT_IMPLEMENTED();
}


uint WINAPI GetFileType(uint h)
{
	DBG("h=%d", h );
	return _GetHandleType(h);
}


void WINAPI GetFileAttributesA()
{
	NOT_IMPLEMENTED();
}


void WINAPI GetFileInformationByHandle()
{
	NOT_IMPLEMENTED();
}


void WINAPI GetFullPathNameA()
{
	NOT_IMPLEMENTED();
}


void WINAPI DeviceIoControl()
{
	NOT_IMPLEMENTED();
}


void WINAPI FlushFileBuffers()
{
	NOT_IMPLEMENTED();
}


void WINAPI GetOverlappedResult()
{
	NOT_IMPLEMENTED();
}


void WINAPI ReadFile()
{
	NOT_IMPLEMENTED();
}
