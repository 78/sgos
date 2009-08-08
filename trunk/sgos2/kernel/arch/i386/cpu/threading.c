#include <sgos.h>
#include <arch.h>
#include <debug.h>
#include <thread.h>
#include <process.h>
#include <string.h>
#include <mm.h>

static TSS g_tss;
static SEGMENT_DESC g_ldt[3];	//局部描述符 0x7  0xF  0x17

#define GET_THREAD_REGS(p) (I386_REGISTERS*)((t_32)p+sizeof(THREAD)-sizeof(I386_REGISTERS) )

extern void enter_user_mode();
extern THREAD_BOX tbox;
void start_threading()
{
	//初始化2个局部描述符, LDT
	//用户
	set_ldt_desc( &g_ldt[0], 0, 0, 0 );	// dummy
	set_ldt_desc( &g_ldt[1], 0, 0xFFFFF, DA_CR | DA_32 | DA_LIMIT_4K /*| DA_DPL3*/ );	// 32位代码段	段限4GB
	set_ldt_desc( &g_ldt[2], 0, 0xFFFFF, DA_DRW | DA_32 | DA_LIMIT_4K /*| DA_DPL3*/ );	// 可读写数据段	段限4GB
	
	//TSS, 保存着内核所用的堆栈, 从特权3到特权0要借助TSS
	memset(&g_tss, 0, sizeof(TSS) );
	g_tss.ss0 = 0x10;
	g_tss.esp0 = (uint)current_thread()+sizeof(THREAD);
	g_tss.iobase = sizeof(TSS);
	//设置TSS描述符
	set_gdt_desc( 5, (t_32)&g_tss, sizeof(TSS)-1, DA_386TSS );
	//设置LDT描述符
	set_gdt_desc( 6, (t_32)g_ldt, sizeof(SEGMENT_DESC)*3-1, DA_LDT );
	//加载TSS和LDT
	__asm__ __volatile__("mov $0x28, %bx\n\t"
		"ltr %bx\n\t"
		"mov $0x30, %bx\n\t"
		"lldt %bx\n" );
	enter_threading_mode();
	tbox.running = current_proc()->thread;
	//back here
	kinit_resume();
}

void i386_switch( THREAD*, uint*, uint* );
void switch_to( THREAD* cur, THREAD* thr )
{
	//let clock work
	out_byte(0x20, 0x20);
	i386_switch( cur, &cur->stack_pointer, &thr->stack_pointer );
}

//初始化线程的寄存器信息。
void init_thread_regs( THREAD* thr, THREAD* parent,
	void* context, uint entry_addr, uint stack_addr )
{
	I386_REGISTERS *r;
	if( !parent )
		return;
	r = GET_THREAD_REGS(thr);
	r->gs = r->fs = r->es = r->ds = r->ss = 0x14;
	r->cs = 0x0C;
	r->eflags = 0x203;
	r->esp = stack_addr;
	r->kesp = 0;
	r->eip = entry_addr;
	thr->stack_pointer = (t_32)r;
//	PERROR("ok init:%d  pointer:0x%X", thr->id, thr->stack_pointer );
}
