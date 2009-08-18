
.global _i386_switch
_i386_switch:	;// 线程切换
	pushl	%ebp
	movl	%esp, %ebp
	pushfl
	push	%cs
	push	$return_from_switch
	subl	$8, %esp
	pushal
	pushl	%ds
	push	%es
	push	%fs
	push	%gs
	movl	0x0C(%ebp), %edi
	movl	%esp, (%edi)	;// cur->stack_pointer = esp
	movl	0x10(%ebp), %esp
	movl	(%esp), %esp	;// esp = p->stack_pointer
	pop	%gs
	pop	%fs
	pop	%es
	pop	%ds
	popal
	addl	$8, %esp
	iret

return_from_switch:
	pop	%ebp
	ret


