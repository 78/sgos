// Ported from SGOS1

#include <sgos.h>
#include <ctype.h>
#include <debug.h>
#include <string.h>
#include <stdarg.h>
#include <arch.h>
#include <terminal.h>
#include <mutex.h>
#include <stdlib.h>
#include <mm.h>
#include <thread.h>

//#define KEEP_INTEGRAL_INFO

static int putstr(const char* str);
static char* ksyms;
static int (*printer)(const char*);

// this should be called as early as possible..
void debug_init()
{
	printer = putstr;
	ksyms = NULL;
	clrscr();
	printer("System Debugger( 2009-08-04 ) for SGOS2.\n");
}

// note: would use kmalloc
// mm must be inited 
void debug_set_symbol( size_t beg, size_t end )
{
	size_t len = end-beg;
	ksyms = kmalloc( len+1 );
	if( !ksyms )
		return;
	strncpy ( ksyms, (char*)beg, len );
	PERROR("ok");
}

// 打印堆栈信息
void debug_dump_stack(  void * thr, uint stk )
{
	size_t start, end=(uint)thr+ sizeof(THREAD);
	char tmp[16], *p, *q;
	size_t i;
	GET_STACK_POINTER(start);
	kprintf("tid: %d dump stcak at 0x%X to 0x%X: ", ((THREAD*)thr)->tid, start, end);
	if( start>=(size_t)thr && start<=end-sizeof(size_t) )
	{
		size_t value;
		for( ; start<=end-sizeof(size_t); start+=sizeof(size_t) )
		{
			value = *((size_t*)start);
			if( value>=0xC0100000 && value <=0xC0110000 ){
				sprintf( tmp, "%x", value );
				for( i=value; i>=0xC0100000; i-=1 ){
					sprintf( tmp, "%x", i );
					if( (p=strstr( ksyms, tmp )) != NULL ){
						p+=11;
						q = strchr( p, '\n' );
						if( q )	*q='\0';
						kprintf("[%s+%x]", p, value-i );
						if( q )	*q='\n';
						break;
					}
				}
				if( i<0xC0100000 ){
					kprintf("[%x]", value );
				}
			}else{
				kprintf("(%x)", value );
			}
		}
	}
	kprintf("\n");
}

// generally use PERROR instead of kprintf
void kprintf(const char *fmt, ...)
{
	char printbuf[256];
	va_list args;
	int i;
#ifdef KEEP_INTEGRAL_INFO
	uint flags;
	va_start(args, fmt);
	i=vsprintf( printbuf, fmt, args );
	local_irq_save(flags);
	printer( printbuf );
	local_irq_restore(flags);
#else
	va_start(args, fmt);
	i=vsprintf( printbuf, fmt, args );
	printer( printbuf );
#endif
	va_end(args);
}

// 内核停止工作
void die(const char *s )
{
	debug_print( (char*)s );
	local_irq_disable();
	while(1)
		halt();
}

//打印调试信息
int debug_print( char *buf )
{
	return printer(buf);
}

//默认终端输出
static int putstr( const char* buf )
{
	int i=0;
	while(buf[i])
		putchar(buf[i++]);
	return i;
}

