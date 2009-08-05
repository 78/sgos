/*
 * Huang Guan
 * 090803
 */

.section .init

;// same as we set in linker script
K_VIR_ADDR =			0xC0000000
K_PHYS_START=			0x100000
PAGE_SIZE=				(1<<12) ;//4KB

P_PRESENT=				0x01
P_WRITE=				0x02

PDE_ADDR=				0x10000

;//multiboot defines
MULTIBOOT_PAGE_ALIGN=	(1 << 0)	;//modules are page aligned
MULTIBOOT_MEMORY_INFO=	(1 << 1)
MULTIBOOT_VIDEO_INFO=	(1 << 2)
MULTIBOOT_AOUT_KLUDGE=  (1 << 16)

MULTIBOOT_HEADER_MAGIC=	(0x1BADB002)
MULTIBOOT_HEADER_FLAGS=	(MULTIBOOT_PAGE_ALIGN | MULTIBOOT_MEMORY_INFO | MULTIBOOT_AOUT_KLUDGE | MULTIBOOT_VIDEO_INFO)
MULTIBOOT_CHECKSUM=		-(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)


.global		_start
.global		_multiboot_header
.extern		K_STACK_ADDR
.extern		_multiboot_init
_start:
	jmp		multiboot_entry
.align 4
_multiboot_header:
	.long 	MULTIBOOT_HEADER_MAGIC
	.long	MULTIBOOT_HEADER_FLAGS
	.long	MULTIBOOT_CHECKSUM
	.long	_multiboot_header -K_VIR_ADDR	/* header addr */
	.long	(_start - K_VIR_ADDR)
	.long	0
	.long	0
	.long	(multiboot_entry - K_VIR_ADDR)
	.long	0	;// video mode type
	.long	0	;// width
	.long	0	;// height
	.long	0	;// depth
meminfo_addr:
	.long	0

multiboot_info:
	.long	0

multiboot_entry:
	/* save multiboot info */
	movl	%ebx, (multiboot_info - K_VIR_ADDR)
	
	;//借助物理地址 [0x10000,0x13000) 来做一个临时分页表，当内存模块初始化之后，这里作废。
	;//填充2个页表，映射物理内存前8MB
	movl	$0x3, %eax		;//P_PRESENT | P_WRITE
	movl	$(1024 * 2), %ecx
	movl	$0x11000, %edi
MAP_PTE:
	stosl
	add		$PAGE_SIZE, %eax
	loop	MAP_PTE

	;//虚拟地址 [0,0x400000)
	movl	$(0x11000 | P_PRESENT | P_WRITE), PDE_ADDR
	
	;//[0xC0000000,0xC0400000)
	movl	$(0x11000 | P_PRESENT | P_WRITE), PDE_ADDR + (K_VIR_ADDR >> 20) 
	
	;//设置当前页目录
	movl	$0x10000, %eax
	movl	%eax, %cr3

	;//开启分页
	movl	%cr0, %eax
	orl		$0x80010000, %eax ;//PG|WP
	movl	%eax, %cr0
	
	;//加载gdt
	lgdtl	gdtr
	ljmp	$0x08, $1f
1:
	movw	$0x10, %ax
	movw	%ax, %ds
	movw	%ax, %es
	movw	%ax, %fs
	movw	%ax, %gs
	movw	%ax, %ss
	;//设置内核堆栈
	movl	$0x10000, %esp
	movl	multiboot_info, %ebx
	pushl	%ebx
	call	_multiboot_init
loop:
	hlt
	jmp		loop

.align 4
;//	GDTR
gdtr:
	.word	(gdt_end - gdt_beg - 1)
	.long	(gdt_beg - K_VIR_ADDR)

.align 4
gdt_beg:
	;//the first gdt is all zeros
	.long	0
	.long	0
	
	;//code descriptor: 0x8 4GB flat memory
	.word	0xFFFF		;//limit
	.word	0			;//base
	.byte	0			;//base
	.byte	0x9A		;//CODE|PRESENT|NON_CONFORM|EXEC 
	.byte	0xCF		;//G=D=1 AVL=0 limit:1111
	.byte	0			;//
	;//data descriptor: 0x10
	
	.word	0xFFFF		;//4GB memory limit 
	.word	0			;//base
	.byte	0			;//base
	.byte	0x92		;//DATA|PRESENT|WRITE 
	.byte	0xCF		;//G=D=1 AVL=0 limit/len:1111
	.byte	0			;//

gdt_end:
