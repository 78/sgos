#ifndef _APIDEF_H_
#define _APIDEF_H_

#include <types.h>
#include <sgos.h>

//system call for SGOS2
//调试
SYSCALL0( 0, uint, Test );
SYSCALL1( 1, int, Print, const char*, buf );
//消息处理
SYSCALL2( 2, int, Send, Message*, msg, time_t, timeout );
SYSCALL2( 3, int, Receive, Message*, msg, time_t, timeout );
SYSCALL1( 4, int, Reply, Message*, msg );

#endif //_APIDEF_H_
