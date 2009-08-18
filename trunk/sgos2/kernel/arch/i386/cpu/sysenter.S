;// perform a faster system call
;// 090818 implemeted.

.global _sysenter_handler
ret_val:
.long	0
_sysenter_handler:
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
	call	*_syscall_table(,%eax,4)
	addl	$12, %esp
	movl	%eax, ret_val
	popl	%es
	popl	%ds
	popal
	movl	ret_val, %eax
	movl	%ebp, %ecx
	movl	%esi, %edx
	sysexit

