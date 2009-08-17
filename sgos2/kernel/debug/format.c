//ported from SGOS1

#include <stdarg.h>
#include <debug.h>
#include <string.h>
#include <thread.h>

void print_err(char* file, char* function, int line, const char *fmt, ...)
{
	va_list args;
	char printbuf[256];
	int i;
	va_start(args, fmt);
	i=vsprintf( printbuf, fmt, args );
	va_end(args);
	kprintf("[%s]%s(%d): %s\n", file, function, line, printbuf );
}


void kernel_err(char* file, char* function, int line, const char *fmt, ...)
{
	va_list args;
	char printbuf[256];
	int i;
	THREAD* thr;
	va_start(args, fmt);
	i=vsprintf( printbuf, fmt, args );
	va_end(args);
	kprintf("[%s]%s(%d): %s\n", file, function, line, printbuf );
	thr = current_thread();
	if( thr )
		thread_terminate( thr, -1 );
	else
		die("System halted.");
}

