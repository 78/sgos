//ported from SGOS1

#include <stdarg.h>
#include <stdlib.h>
#include <kd.h>
#include <tm.h>
#include <mm.h>

//发出错误警告
void print_err(char* file, char* function, int line, const char *fmt, ...)
{
	va_list args;
	char printbuf[256];
	int i;
	va_start(args, fmt);
	i=vsprintf( printbuf, fmt, args );
	va_end(args);
	KdPrintf("[%s]%s(%d): %s\n", file, function, line, printbuf );
}

//Fatal Error 严重错误
void kernel_err(char* file, char* function, int line, const char *fmt, ...)
{
	va_list args;
	char printbuf[256];
	int i;
	KThread* thr;
	va_start(args, fmt);
	i=vsprintf( printbuf, fmt, args );
	va_end(args);
	KdPrintf("[%s]%s(%d): %s\n", file, function, line, printbuf );
	thr = TmGetCurrentThread();
	if( thr ){
		TmTerminateThread( thr, -1 );
	}else
		KeBugCheck("System halted.");
}

//诊断错误
void assert_err(char* file, char* function, int line, int b )
{
	KThread* thr;
	if( b )
		return;
	KdPrintf("[%s]%s(%d): Assertion failed.\n", file, function, line );
	thr = TmGetCurrentThread();
	if( thr ){
		KdPrintf("Terminated. tid:%d SpaceId:%d\n", thr->ThreadId, thr->Space->SpaceId );
		TmTerminateThread( thr, -1 );
	}else
		KeBugCheck("System halted.");
}

