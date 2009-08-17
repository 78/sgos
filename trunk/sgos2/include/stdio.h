#ifndef _STDIO_H_
#define _STDIO_H_

#ifndef EXTERN
#ifdef __cplusplus 
#define EXTERN extern "C" 
#else
#define EXTERN extern
#endif
#endif

#define max(a,b) (a>b?a:b)
#define min(a,b) (a>b?b:a)

#include <types.h>
#include <string.h>
int printf(const char*, ... );


#endif
