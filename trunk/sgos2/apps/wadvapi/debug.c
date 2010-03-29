/*
 *  debug.c
 *
 *  SGOS debugger.
 *
 *  Copyright (C) 2008  Huang Guan
 *
 *  2008-01-31 Created.
 *
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

void print_error(char* file, char* function, int line, const char *fmt, ...)
{
	va_list args;
	char printbuf[512];
	int i;
	va_start(args, fmt);
	i=vsprintf( printbuf, fmt, args );
	printbuf[i] = 0;
	va_end(args);
	printf("[%s]%s(%d): %s\n", file, function, line, printbuf);
}

