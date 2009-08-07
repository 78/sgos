.global _enter_threading_mode

_enter_threading_mode:
	pushl	$0x14	;//$0x17	;//用户堆栈段
	pushl	%esp	;//用户堆栈指针
	pushl	$0x202	;//flag寄存器
	pushl	$0x0C	;//0x0F用户代码段
	pushl	$return
	mov		$0x14, %ax	;//$0x17
	mov		%ax, %es
	mov		%ax, %ds
	mov		%ax, %fs
	mov		%ax, %gs
	iret

return:
	addl	$8, %esp
	ret


.global _i386_switch
_i386_switch:	;// 线程切换
	pushl	%ebp
	movl	%esp, %ebp
	pushf
	push	%cs
	push	$return_from_switch
	subl	$8, %esp
	pusha
	pushl	%ds
	push	%es
	push	%fs
	push	%gs
	movl	0x0C(%ebp), %edi
	movl	%esp, (%edi)	;// cur->stack_pointer = esp
	movl	0x10(%ebp), %esp
	movl	(%esp), %esp	;// esp = p->stack_pointer
	pop		%gs
	pop		%fs
	pop		%es
	pop		%ds
	popa
	addl	$8, %esp
	iret

return_from_switch:
	pop		%ebp
	ret


