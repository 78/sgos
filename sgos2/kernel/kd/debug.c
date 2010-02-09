// Ported from SGOS1

#include <sgos.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <kd.h>
#include <arch.h>
#include <terminal.h>
#include <ipc.h>
#include <mm.h>
#include <tm.h>

//#define KEEP_INTEGRAL_INFO

static int PrintString(const char* str);
static int (*Printer)(const char*);

// this should be called as early as possible..
void KdInitializeDebugger()
{
	Printer = PrintString;
	ArClearScreen();
	Printer("System Debugger( 2009-08-04 ) for SGOS2.\n");
}

// generally use PERROR instead of KdPrintf
void KdPrintf(const char *fmt, ...)
{
	char printbuf[256];
	va_list args;
	int i;
#ifdef KEEP_INTEGRAL_INFO
	uint flags;
	va_start(args, fmt);
	i=vsprintf( printbuf, fmt, args );
	ArLocalSaveIrq(flags);
	KdPrint( printbuf );
	ArLocalRestoreIrq(flags);
#else
	va_start(args, fmt);
	i=vsprintf( printbuf, fmt, args );
	KdPrint( printbuf );
#endif
	va_end(args);
}

//打印调试信息
int KdPrint( char *buf )
{
	return Printer(buf);
}

//默认终端输出
static int PrintString( const char* buf )
{
	int i=0;
	while(buf[i])
		ArPrintChar(buf[i++]);
	return i;
}

