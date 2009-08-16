//API for SGOS2

#define SYSCALL0(id, type,name) \
type __stdcall sys_##name(void) \
{ \
	long result; \
	__asm__ volatile ("int $0xA1" \
	: "=a" (result) \
	: "a"(id), "c"(0)  ); \
return (type) result; \
}

#define SYSCALL1(id, type,name,atype,a) \
type __stdcall sys_##name(atype a) \
{ \
	long result; \
	__asm__ volatile ("int $0xA0" \
	: "=a" (result) \
	: "a"(id), "b"(a)  ); \
	return (type) result; \
}

#define SYSCALL2(id, type,name,atype,a,btype,b) \
type __stdcall sys_##name(atype a,btype b) \
{ \
	long result; \
	__asm__ volatile ("int $0xA0" \
	: "=a" (result) \
	: "a"(id), "b"(a), "c"(b) ); \
	return (type) result; \
}

#define SYSCALL3(id, type,name,atype,a,btype,b,ctype,c) \
type __stdcall sys_##name(atype a,btype b,ctype c) \
{ \
	long result; \
	__asm__ volatile ("int $0xA0" \
	: "=a" (result) \
	: "a"(id), "b"(a), "c"(b), "d"(c) ); \
	return (type) result; \
}

#include <apidef.h>
