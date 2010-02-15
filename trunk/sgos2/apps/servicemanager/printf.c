
#include <stdarg.h>
#include <api.h>
#include <stdio.h>

int printf(const char *fmt, ...)
{
	va_list args;
	int i;
	char buffer[1024];
	va_start(args, fmt);
	i=vsprintf(buffer,fmt,args);
	Api_Print(buffer);
	va_end(args);
	return i;
}


