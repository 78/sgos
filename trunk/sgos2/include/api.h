#ifndef _API_H_
#define _API_H_

#ifndef EXTERN
#ifdef __cplusplus
#define EXTERN extern "C"
#else
#define EXTERN
#endif
#endif

#define SYSCALL0(id, type, name) EXTERN type __stdcall sys_##name()
#define SYSCALL1(id, type, name, atype, a) EXTERN type __stdcall sys_##name( atype a )
#define SYSCALL2(id, type, name, atype, a, btype, b) EXTERN type __stdcall sys_##name( atype a, btype b )
#define SYSCALL3(id, type, name, atype, a, btype, b, ctype, c) EXTERN type __stdcall sys_##name( atype a, btype b, ctype c )

#include <apidef.h>

#endif
