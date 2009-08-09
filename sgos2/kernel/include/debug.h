//090804 Ported from SGOS1

#ifndef _DEBUG_H_
#define _DEBUG_H_

#define PERROR(args ...) print_err( __FILE__, __FUNCTION__, __LINE__, ##args )
#define KERROR(args ...) kernel_err( __FILE__, __FUNCTION__, __LINE__, ##args )

void die( const char *s );	//内核死亡
void kprintf(const char *fmt, ...);	//打印字符信息
int debug_print( char *buf );	//同上,只是不能格式化字符串
void debug_init();	//初始化字符调试工具

#endif
