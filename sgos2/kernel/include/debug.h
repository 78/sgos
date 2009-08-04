//090804 Ported from SGOS1

#ifndef _DEBUG_H_
#define _DEBUG_H_

#define PERROR(args ...) print_err( __FILE__, __FUNCTION__, __LINE__, ##args )
#define KERROR(args ...) kernel_err( __FILE__, __FUNCTION__, __LINE__, ##args )

void die( const char *s );
void kprintf(const char *fmt, ...);
int debug_print( char *buf );
void debug_init();

#endif
