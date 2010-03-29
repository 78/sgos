#ifndef _DEBUG_H
#define _DEBUG_H

#include <stdio.h>
#include <sgos.h>

//#define RELEASE

#ifndef RELEASE
#define DBG(args ...) \
	print_error( __FILE__, (char*)__func__, __LINE__, ##args )
#else
#define DBG(args ...) 
//#define DBG printf
#endif
#define MSG	printf
void print_error(char* file, char* function, int line, const char *fmt, ...);

#define NOT_IMPLEMENTED() DBG("## Not implemented.")

#endif //_DEBUG_H

