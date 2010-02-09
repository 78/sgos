#ifndef _API_H_
#define _API_H_

#include <sgos.h>

#ifndef EXTERN
#ifdef __cplusplus
#define EXTERN extern "C"
#else
#define EXTERN
#endif
#endif

#define SYSCALL0(id, type, name) EXTERN type Api_##name()
#define SYSCALL1(id, type, name, atype, a) EXTERN type Api_##name( atype a )
#define SYSCALL2(id, type, name, atype, a, btype, b) EXTERN type Api_##name( atype a, btype b )
#define SYSCALL3(id, type, name, atype, a, btype, b, ctype, c) EXTERN type Api_##name( atype a, btype b, ctype c )
#define SYSCALL4(id, type, name, atype, a, btype, b, ctype, c, dtype, d) EXTERN type Api_##name( atype a, btype b, ctype c, dtype d )

#include "../kernel/include/apidef.h"

//init.c
ThreadInformation* GetThreadInformation();
ProcessInformation* GetProcessInformation();

#endif
