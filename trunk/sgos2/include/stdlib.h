#ifndef _STDLIB_H_
#define _STDLIB_H_

#include <types.h>
#include <stdarg.h>

void*	malloc(size_t);
void	free(void*);
void*	calloc(size_t, size_t);
void* 	realloc(void*, size_t);

int sprintf(char * buf, const char *fmt, ...);
int vsprintf(char *buf, const char *fmt, va_list args);

#endif

