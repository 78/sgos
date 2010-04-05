#ifndef _APIDEF_H_
#define _APIDEF_H_

#include <types.h>
#include <sgos.h>

//system call for SGOS2
//调试
SYSCALL0( 0, uint, Test );
SYSCALL2( 1, int, Print, const char*, buf, int, nbytes );
//消息处理
SYSCALL2( 2, int, Send, Message*, msg, time_t, timeout );
SYSCALL2( 3, int, Receive, Message*, msg, time_t, timeout );
SYSCALL3( 4, int, InvokeBiosService, int, int_no, void*, context, size_t, context_size );

#endif //_APIDEF_H_
