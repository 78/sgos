//090804 Ported from SGOS1

#ifndef _DEBUG_H_
#define _DEBUG_H_

#include <types.h>

#define ASSERT(condition) assert_err( (char*)__FILE__, (char*)__FUNCTION__, __LINE__, condition )
#define PERROR(args ...) print_err( (char*)__FILE__, (char*)__FUNCTION__, __LINE__, ##args )
#define KERROR(args ...) kernel_err( (char*)__FILE__, (char*)__FUNCTION__, __LINE__, ##args )

void print_err(char* file, char* function, int line, const char *fmt, ...);
void kernel_err(char* file, char* function, int line, const char *fmt, ...);
void assert_err(char* file, char* function, int line, int b);
void die( const char *s );	//内核死亡
void kprintf(const char *fmt, ...);	//打印字符信息
int debug_print( char *buf );	//同上,只是不能格式化字符串
void debug_init();	//初始化字符调试工具
void debug_dump_stack( void* thr, size_t stk );
void debug_set_symbol( size_t beg, size_t end );

#endif
