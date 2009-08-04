;//ported from SGOS1
;//System call
.global _syscall_interrupt
_syscall_interrupt:
	cli
.loop:
	hlt
	jmp .loop


;//-----------------------------------------------
;// IRQ中断响应
;//-----------------------------------------------
.global _irq0
.global _irq1
.global _irq2
.global _irq3
.global _irq4
.global _irq5
.global _irq6
.global _irq7
.global _irq8
.global _irq9
.global _irq10
.global _irq11
.global _irq12
.global _irq13
.global _irq14
.global _irq15

;// 32: IRQ0
_irq0:
	cli
	push	$0
	push	$32
	jmp		do_irq

;// 33: IRQ1
_irq1:
	cli
	push	$0
	push	$33
	jmp		do_irq

;// 34: IRQ2
_irq2:
	cli
	push	$0
	push	$34
	jmp		do_irq

;// 35: IRQ3
_irq3:
	cli
	push	$0
	push	$35
	jmp		do_irq

;// 36: IRQ4
_irq4:
	cli
	push	$0
	push	$36
	jmp		do_irq

;// 37: IRQ5
_irq5:
	cli
	push	$0
	push	$37
	jmp		do_irq

;// 38: IRQ6
_irq6:
	cli
	push	$0
	push	$38
	jmp		do_irq

;// 39: IRQ7
_irq7:
	cli
	push	$0
	push	$39
	jmp		do_irq

;// 40: IRQ8
_irq8:
	cli
	push	$0
	push	$40
	jmp		do_irq

;// 41: IRQ9
_irq9:
	cli
	push	$0
	push	$41
	jmp		do_irq

;// 42: IRQ10
_irq10:
	cli
	push	$0
	push	$42
	jmp		do_irq

;// 43: IRQ11
_irq11:
	cli
	push	$0
	push	$43
	jmp		do_irq

;// 44: IRQ12
_irq12:
	cli
	push	$0
	push	$44
	jmp		do_irq

;// 45: IRQ13
_irq13:
	cli
	push	$0
	push	$45
	jmp		do_irq

;// 46: IRQ14
_irq14:
	cli
	push	$0
	push	$46
	jmp		do_irq

;// 47: IRQ15
_irq15:
	cli
	push	$0
	push	$47
	jmp		do_irq


.extern _irq_handler
do_irq:
	pusha
	pushl	%ds
	pushl	%es
	pushl	%fs
	pushl	%gs

	mov		$0x10, %ax
	mov		%ax, %ds
	mov		%ax, %es

	movl	%esp, %eax

	pushl	%eax
	call	_irq_handler
	popl	%eax

	pop		%gs
	pop		%fs
	pop		%es
	pop		%ds
	popa
	addl	%esp, 8
	iret


;//-----------------------------------------------
;// 陷阱调用过程 
;//-----------------------------------------------
;// 2006-5-2 15:42
;// In just a few pages in this tutorial, we will add our Interrupt
;// Service Routines (ISRs) right here!
.global _isr0
.global _isr1
.global _isr2
.global _isr3
.global _isr4
.global _isr5
.global _isr6
.global _isr7
.global _isr8
.global _isr9
.global _isr10
.global _isr11
.global _isr12
.global _isr13
.global _isr14
.global _isr15
.global _isr16
.global _isr17
.global _isr18
.global _isr19
.global _isr20
.global _isr21
.global _isr22
.global _isr23
.global _isr24
.global _isr25
.global _isr26
.global _isr27
.global _isr28
.global _isr29
.global _isr30
.global _isr31

;//  0: Divide By Zero Exception
_isr0:
    cli
    pushl	$0
    pushl	$0
    jmp		do_isr

;//  1: Debug Exception
_isr1:
    cli
    pushl	$0
    pushl	$1
    jmp		do_isr

;//  2: Non Maskable Interrupt Exception
_isr2:
    cli
    pushl	$0
    pushl	$2
    jmp		do_isr

;//  3: Int 3 Exception
_isr3:
    cli
    pushl	$0
    pushl	$3
    jmp		do_isr

;//  4: INTO Exception
_isr4:
    cli
    pushl	$0
    pushl	$4
    jmp		do_isr

;//  5: Out of Bounds Exception
_isr5:
    cli
    pushl	$0
    pushl	$5
    jmp		do_isr

;//  6: Invalid Opcode Exception
_isr6:
    cli
    pushl	$0
    pushl	$6
    jmp		do_isr

;//  7: Coprocessor Not Available Exception
_isr7:
    cli
    pushl	$0
    pushl	$7
    jmp		do_isr

;//  8: Double Fault Exception (With Error Code!)
_isr8:
    cli
    pushl	$8
    jmp		do_isr

;//  9: Coprocessor Segment Overrun Exception
_isr9:
    cli
    pushl	$0
    pushl	$9
    jmp		do_isr

;// 10: Bad TSS Exception (With Error Code!)
_isr10:
    cli
    pushl	$10
    jmp		do_isr

;// 11: Segment Not Present Exception (With Error Code!)
_isr11:
    cli
    pushl	$11
    jmp		do_isr

;// 12: Stack Fault Exception (With Error Code!)
_isr12:
    cli
    pushl	$12
    jmp		do_isr

;// 13: General Protection Fault Exception (With Error Code!)
_isr13:
    cli
    pushl	$13
    jmp		do_isr

;// 14: Page Fault Exception (With Error Code!)
_isr14:
    cli
    pushl	$14
    jmp		do_isr

;// 15: Reserved Exception
_isr15:
    cli
    pushl	$0
    pushl	$15
    jmp		do_isr

;// 16: Floating Point Exception
_isr16:
    cli
    pushl	$0
    pushl	$16
    jmp		do_isr

;// 17: Alignment Check Exception
_isr17:
    cli
    pushl	$0
    pushl	$17
    jmp		do_isr

;// 18: Machine Check Exception
_isr18:
    cli
    pushl	$0
    pushl	$18
    jmp		do_isr

;// 19: Reserved
_isr19:
    cli
    pushl	$0
    pushl	$19
    jmp		do_isr

;// 20: Reserved
_isr20:
    cli
    pushl	$0
    pushl	$20
    jmp		do_isr

;// 21: Reserved
_isr21:
    cli
    pushl	$0
    pushl	$21
    jmp		do_isr

;// 22: Reserved
_isr22:
    cli
    pushl	$0
    pushl	$22
    jmp		do_isr

;// 23: Reserved
_isr23:
    cli
    pushl	$0
    pushl	$23
    jmp		do_isr

;// 24: Reserved
_isr24:
    cli
    pushl	$0
    pushl	$24
    jmp		do_isr

;// 25: Reserved
_isr25:
    cli
    pushl	$0
    pushl	$25
    jmp		do_isr

;// 26: Reserved
_isr26:
    cli
    pushl	$0
    pushl	$26
    jmp		do_isr

;// 27: Reserved
_isr27:
    cli
    pushl	$0
    pushl	$27
    jmp		do_isr

;// 28: Reserved
_isr28:
    cli
    pushl	$0
    pushl	$28
    jmp		do_isr

;// 29: Reserved
_isr29:
    cli
    pushl	$0
    pushl	$29
    jmp		do_isr

;// 30: Reserved
_isr30:
    cli
    pushl	$0
    pushl	$30
    jmp		do_isr

;// 31: Reserved
_isr31:
    cli
    pushl	$0
    pushl	$31
    jmp		do_isr


;// isr handler
.extern _isr_handler
do_isr:
	pusha
	pushl	%ds
	pushl	%es
	pushl	%fs
	pushl	%gs

	mov		$0x10, %ax
	mov		%ax, %ds
	mov		%ax, %es

	movl	%esp, %eax

	pushl	%eax
	call	_isr_handler
	popl	%eax

	pop		%gs
	pop		%fs
	pop		%es
	pop		%ds
	popa
	addl	%esp, 8
	iret

