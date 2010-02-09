;// perform a faster system call
;// 090818 implemeted.

.global _ArAsmSysenterHandler
ret_val:
.long	0
_ArAsmSysenterHandler:
	pushal
	pushl	%ds
	pushl	%es
	pushl	%eax
	movw	$0x10, %ax
	movw	%ax, %ds
	movw	%ax, %es
	popl	%eax
	pushl	%edx
	pushl	%ecx
	pushl	%ebx
	sti
	call	*_SystemCallTable(,%eax,4)
	addl	$12, %esp
	movl	%eax, ret_val
	popl	%es
	popl	%ds
	popal
	movl	ret_val, %eax
	movl	%ebp, %ecx
	movl	%esi, %edx
	sysexit

