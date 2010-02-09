.global _ArAsmEnterThreadingMode
;// 主要是变更内核堆栈
_ArAsmEnterThreadingMode:
	pushl	%ebp
	movl	%esp, %ebp
	movl	0x08(%ebp), %esp
	pushl	0x04(%ebp)	;//returen address
	pushfl			;//flag寄存器
	pushl	$0x08		;//代码段
	pushl	$return
	movw	$0x10, %ax
	movw	%ax, %es
	movw	%ax, %ds
	iret
return:
	ret

.global _ArAsmSwapContext
_ArAsmSwapContext:	;// 线程切换
	pushl	%ebp
	movl	%esp, %ebp
	pushfl
	push	%cs
	push	$Return
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

Return:
	popl	%ebp
	ret


