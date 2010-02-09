#ifndef _APIDEF_H_
#define _APIDEF_H_

#include <types.h>

//system call for SGOS2
//调试
SYSCALL0( 0, uint, Test );
SYSCALL1( 1, int, Print, const char*, buf );
//消息处理
/*
SYSCALL1( 2, int, Send, void*, content );
SYSCALL1( 3, int, Call, void*, content );
SYSCALL1( 4, int, CallTimeout, void*, content );
SYSCALL1( 5, int, Receive, void*, content );
SYSCALL1( 6, int, ReceiveTimeout, void*, content );
SYSCALL1 7, int, Reply, uint, result );
*/

#endif //_APIDEF_H_
