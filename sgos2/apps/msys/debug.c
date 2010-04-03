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
#include <api.h>

void print_error(char* file, char* function, int line, const char *fmt, ...)
{
	va_list args;
	char printbuf[1024];
	int i;
	va_start(args, fmt);
	i=vsprintf( printbuf, fmt, args );
	printbuf[i] = 0;
	va_end(args);
	Api_Print( "[", 1);
	Api_Print( file, strlen(file) );
	Api_Print( "]", 1);
	Api_Print( function, strlen(function) );
	Api_Print( ": ", 2);
	Api_Print( printbuf, i );
	Api_Print( "\n", 1);
}

void ASSERT(int a)
{
	if( !a ){
		Api_Print("Assertion failed at somewhere.\n", 100);
	}
}
