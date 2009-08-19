// Ported from SGOS1

#include <sgos.h>
#include <ctype.h>
#include <debug.h>
#include <string.h>
#include <stdarg.h>
#include <arch.h>
#include <terminal.h>
#include <mutex.h>

static int putstr(const char* str);

static int (*printer)(const char*);

// this should be called as early as possible..
void debug_init()
{
	printer = putstr;
	clrscr();
	printer("System Debugger( 2009-08-04 ) for SGOS2.\n");
}

// generally use PERROR instead of kprintf
void kprintf(const char *fmt, ...)
{
	char printbuf[256];
	va_list args;
	int i;
	uint eflags;
	va_start(args, fmt);
	i=vsprintf( printbuf, fmt, args );
	//为了保持调试信息的完整，先关中断。
	local_irq_save(eflags);
	printer( printbuf );
	local_irq_restore(eflags);
	va_end(args);
}

void die(const char *s )
{
	debug_print( (char*)s );
	local_irq_disable();
	while(1)
		halt();
}

int debug_print( char *buf )
{
	return printer(buf);
}

static int putstr( const char* buf )
{
	int i=0;
	while(buf[i])
		putchar(buf[i++]);
	return i;
}

