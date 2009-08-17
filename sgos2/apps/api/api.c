//API for SGOS2

#define SYSCALL0(id, type,name) \
type sys_##name(void) \
{ \
	long result; \
	__asm__ __volatile__ ("int $0xA1" \
	: "=a" (result) \
	: "a"(id), "c"(0)  ); \
return (type) result; \
}

#define SYSCALL1(id, type,name,atype,a) \
type sys_##name(atype a) \
{ \
	long result; \
	__asm__ __volatile__ ("int $0xA1" \
	: "=a" (result) \
	: "a"(id), "b"(a)  ); \
	return (type) result; \
}

#define SYSCALL2(id, type,name,atype,a,btype,b) \
type sys_##name(atype a,btype b) \
{ \
	long result; \
	__asm__ __volatile__ ("int $0xA1" \
	: "=a" (result) \
	: "a"(id), "b"(a), "c"(b) ); \
	return (type) result; \
}

#define SYSCALL3(id, type,name,atype,a,btype,b,ctype,c) \
type sys_##name(atype a,btype b,ctype c) \
{ \
	long result; \
	__asm__ __volatile__ ("int $0xA1" \
	: "=a" (result) \
	: "a"(id), "b"(a), "c"(b), "d"(c) ); \
	return (type) result; \
}

#include "../../kernel/include/apidef.h"

//线程用户态入口点
//内核会把用户态入口地址放在ebx寄存器里。(x86)
void _program_entry_()
{
	__asm__ __volatile__(
		"call *%ebx;"
		"movl %eax, %ebx;"
		"movl $6, %eax;"
		"int $0xA1" );
}

