/*
	The better way to do this is to touch every page 
	when we decrement the stack pointer. 
	Note: gcc will store the size in eax.
*/
.global __alloca
__alloca:
	pushl %edx
	movl %esp, %edx
	addl $0x8, %edx
	subl %eax, %edx
	movl %esp, %eax
	movl %edx, %esp
	movl (%eax), %edx
	movl 4(%eax), %eax
	jmp *%eax
