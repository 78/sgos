#ifndef _APIDEF_H_
#define _APIDEF_H_

#include <types.h>

//system call for SGOS2
SYSCALL0( 0, int, test );
SYSCALL1( 1, int, dprint, const char*, buf );
SYSCALL2( 2, int, send, const char*, buf, size_t, len );
SYSCALL3( 3, int, recv, char*, buf, size_t, buf_siz, uint, flag );

#endif //_APIDEF_H_
