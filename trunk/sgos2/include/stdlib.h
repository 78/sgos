#ifndef _STDLIB_H_
#define _STDLIB_H_

#include <sgos.h>
#include <stdarg.h>


EXTERN void*	malloc(size_t);
EXTERN void	free(void*);
EXTERN void*	calloc(size_t, size_t);
EXTERN void* 	realloc(void*, size_t);
EXTERN int rand(void);
EXTERN void srand(int seed);

EXTERN int sprintf(char * buf, const char *fmt, ...);
EXTERN int vsprintf(char *buf, const char *fmt, va_list args);

EXTERN void sleep(unsigned int ms);

#endif

