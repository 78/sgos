#ifndef _ARCH_H_
#define _ARCH_H_

#include <sgos.h>
#include <gdt_const.h>
#include <lock.h>
#include <msr.h>
#include <io.h>

#define RTC_FREQUENCY	200	//200Hz  这是时钟频率，具体在arch/i386/clock/rtc.c

#define SYSTEM_INTERRUPT	0xA1
#define PAGEFAULT_INTERRUPT	14
#define GPF_INTERRUPT		13
#define RTC_INTERRUPT		0
#define FPU_INTERRUPT		7

#define GD_KERNEL_CODE	0x08
#define GD_KERNEL_DATA	0x10
#define GD_USER_CODE	0x1b
#define GD_USER_DATA	0x23
#define GD_USER_TIB	0x3B
#define GD_TSS_INDEX	5
#define GD_TIB_INDEX	7

#define PAGE_SIZE	4096
#define PAGE_SIZE_BITS 12
#define PAGE_ALIGN(a) ((a&0xFFF)?((a&0xFFFFF000)+0x1000):a)
#define KERNEL_BASE	0xC0000000
#define PROC_PAGE_DIR_BASE	0xE0000000
#define PROC_PAGE_DIR_END	0xE0400000
#define PROC_PAGE_TABLE_MAP	0xBFC00000

#define IS_KERNEL_MEMORY(addr) ( addr>=KERNEL_BASE && addr<PROC_PAGE_DIR_END )
#define IS_USER_MEMORY(addr) ( addr <= PROC_PAGE_TABLE_MAP )

#define GET_STACK_POINTER(stk) __asm__ __volatile__("movl %%esp, %%eax" :"=a"(stk):)
#define SET_INTR_GATE(vector, handle) set_gate(vector, DA_386IGate, handle)
#define SET_TRAP_GATE(vector, handle) set_gate( vector, DA_386TGate, handle )

/* VM86 */
#define MAKE_FARPTR(seg,off)	(((uint)(seg)<<16)|(ushort)(off))
#define FARPTR_SEG(fp)	(((uint) fp) >> 16)
#define FARPTR_OFF(fp)	(((uint) fp) & 0xffff)
#define FARPTR_TO_LINEAR(seg,off)	\
	((uint) ((((ushort)(seg)) << 4) + ((ushort)off)))
uint LINEAR_TO_FARPTR(size_t ptr);	//vm86.c
#define EFLAG_IOPL3	0x3000
#define EFLAG_VM	(1<<17)
#define EFLAG_IF	(1<<9)
	
struct THREAD;
// gdt 
// segment
typedef struct SEGMENT_DESC
{
	t_16		limit_low;
	t_16		base_low;
	t_8		base_mid;
	t_8		attr1;
	t_8		limit_high_attr2;
	t_8		base_high;
}SEGMENT_DESC;
// 门描述符
typedef struct GATE_DESC
{
	t_16		offset_low;
	t_16		selector;
	t_8		dcount;	
	t_8		attr;
	t_16		offset_high;
}GATE_DESC;
// GDT位置描述符
typedef struct GDT_ADDR {
	t_16	limit __attribute__((packed));
	t_32	addr __attribute__((packed));
}GDT_ADDR;

//Thread state segment
typedef struct TSS {
	t_32	back_link;
	t_32	esp0;	//kernel esp
	t_32	ss0;
	t_32	esp1;
	t_32	ss1;
	t_32	esp2;
	t_32	ss2;
	t_32	cr3;
	t_32	eip;
	t_32	flags;
	t_32	eax, ecx ,edx, ebx;
	t_32	esp, ebp, esi, edi;
	t_32	es, cs, ss, ds, fs, gs;
	t_32	ldt;
	t_16	trap;
	t_16	iobase;	
	/* I/O位图基址大于或等于TSS段界限，就表示没有I/O许可位图 */
	/*t_8	iomap[2];*/
}TSS;

/*数学协处理器寄存器*/
typedef struct I387_REGISTERS
{
	t_32 cwd;
	t_32 swd;
	t_32 twd;
	t_32 fip;
	t_32 fcs;
	t_32 foo;
	t_32 fos;
	t_32 st[20];
}I387_REGISTERS;

//i386寄存器，用于中断以及线程切换
typedef struct I386_REGISTERS
{
	t_32	gs, fs, es, ds;
	t_32	edi, esi, ebp, kesp, ebx, edx, ecx, eax;
	t_32	int_no, err_code;
	t_32	eip, cs, eflags, esp, ss;
}I386_REGISTERS;	//19*4=76 Bytes

typedef struct I386_CONTEXT{
	t_32	gs, fs, es, ds;
	t_32	edi, esi, ebp, kesp, ebx, edx, ecx, eax;
	t_32	eip, cs, eflags, esp, ss;
}I386_CONTEXT;

typedef struct ARCH_THREAD{
	// 在第一次使用时由内核分配
	I387_REGISTERS*	fsave;
	// 当前是否使用了fpu
	uchar		used_fpu;
	// in_vm86=1时，在线程创建时会初始化vm86寄存器
	uchar		in_vm86;
	// vmflags
	uint		vmflags;
	// interrupt bit
	int		vmflag_if;
}ARCH_THREAD;

//paging
typedef union PAGE_TABLE{
	t_32	v;
	struct{
		unsigned present:1;
		unsigned write:1;
		unsigned user:1;
		unsigned unused1:2;
		unsigned access:1;
		unsigned unused2:2;
		unsigned global:1;
		unsigned share:1;
		unsigned allocated:1;
		unsigned copy_on_write:1;
		unsigned phys_addr:20;
	}a;
}PAGE_TABLE, PAGE_DIR ;
#define P_PRESENT	(1<<0)	//页面存在
#define P_WRITE		(1<<1)	//页面可写
#define P_USER		(1<<2)	//页面为用户级
#define P_ACCESS	(1<<5)	//页面被访问过

#define halt() __asm__("hlt")
#define stts() __asm__ __volatile__("movl %cr0, %eax; orl $8, %eax; movl %eax, %cr0")
#define clts() __asm__ __volatile__("clts")

//gdt
void gdt_init();
void set_gate( int vector, uchar desc_type, void* handle );
void set_gdt_desc( int vector, uint base, uint limit, uint attribute );
void set_idt_desc( int vector, uchar desc_type, void* handler );
void set_ldt_desc( SEGMENT_DESC *desc, uint base, uint limit, uint attribute );
//isr
void isr_init();
void isr_dumpcpu( const I386_REGISTERS *r );
void isr_uninstall( int isr );
int isr_install( int isr, void (*handler)(int err_code, const I386_REGISTERS *r) );
void isr_dumpstack( void* thr, uint stk );
//irq
void irq_init();
void irq_mask( int irq, int enabled );
void irq_uninstall( int irq );
int irq_install( int irq, void (*handler)() );
//syscall
int syscall_interrupt();
//init
int machine_init();
//page
extern uint kernel_page_dir;	//page dir for kernel
int page_init( uint size );
void load_page_dir(uint phys_addr);
void dump_phys_pages();
void free_phys_page( uint addr );
uint get_phys_page();
void reflush_pages();
void load_page_dir(uint phys_addr);
uint switch_page_dir(uint phys_addr);
uint vir_to_phys( uint vir_addr );
void init_page_dir(uint );
//map
uint map_temp_page( uint phys_addr );
void unmap_temp_page( uint vir_addr );
void map_one_page( uint dir, uint vir_addr, uint phys_addr, uint attr );
void unmap_one_page( uint dir, uint vir_addr );
void map_pages( uint dir, uint vir_addr, uint phys_addr, uint size, uint attr );
void unmap_pages( uint dir, uint vir_addr, uint size );
//dir
uint get_page_dir();
void free_page_dir(uint addr);
void dir_init();
//rtc
void rtc_init();
//cpu
void init_thread_regs( struct THREAD* thr, struct THREAD* parent,
	void* context, uint entry_addr, uint stack_addr );
void arch_thread_cleanup( struct THREAD* thr );
void switch_to( struct THREAD* cur, struct THREAD* thr );
void start_threading();
void fastcall_update_esp(uint kesp);
void fastcall_init();
void update_for_next_thread();
void i386_switch( struct THREAD*, uint*, uint* );	//switch.S
void vm86_init();	//vm86.c
int bios_call( int interrupt, void* context, size_t siz );	//vm86.c
//fpu
void fpu_check_and_save( struct THREAD* thr );
void fpu_init();

#endif
