//090804 Ported from SGOS1

#ifndef _KD_H_
#define _KD_H_

#include <types.h>

#define ASSERT(condition) assert_err( (char*)__FILE__, (char*)__FUNCTION__, __LINE__, condition )
#define PERROR(args ...) print_err( (char*)__FILE__, (char*)__FUNCTION__, __LINE__, ##args )
#define KERROR(args ...) kernel_err( (char*)__FILE__, (char*)__FUNCTION__, __LINE__, ##args )

void print_err(char* file, char* function, int line, const char *fmt, ...);
void kernel_err(char* file, char* function, int line, const char *fmt, ...);
void assert_err(char* file, char* function, int line, int b);

void KeBugCheck( const char *s );	//内核死亡
void KdPrintf(const char *fmt, ...);	//打印字符信息
int KdPrint( char *buf );	//同上,只是不能格式化字符串
void KdInitializeDebugger();	//初始化字符调试工具

#endif
