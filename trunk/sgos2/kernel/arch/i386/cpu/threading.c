#include <sgos.h>
#include <arch.h>
#include <debug.h>
#include <thread.h>
#include <process.h>
#include <string.h>
#include <mm.h>

static TSS g_tss;

#define GET_THREAD_REGS(p) (I386_REGISTERS*)((t_32)p+sizeof(THREAD)-sizeof(I386_REGISTERS) )

void start_threading()
{
	//TSS, 保存着内核所用的堆栈, 从特权3到特权0要借助TSS
	memset(&g_tss, 0, sizeof(TSS) );
	g_tss.ss0 = GD_KERNEL_DATA;
	g_tss.esp0 = 0;
	g_tss.iobase = sizeof(TSS);
	//设置TSS描述符
	set_gdt_desc( GD_TSS_INDEX, (t_32)&g_tss, sizeof(TSS)-1, DA_386TSS );
	//加载TSS和LDT
	__asm__ __volatile__("mov $0x28, %bx\n\t"
		"ltr %bx\n\t");
	//开启中断，引发线程切换
	__asm__ __volatile__("sti");
	//继续初始化。
	kinit_resume();
}

//线程切换
void i386_switch( THREAD*, uint*, uint* );
void switch_to( THREAD* cur, THREAD* thr )
{
	//let clock work
	out_byte(0x20, 0x20);
	local_irq_disable();
	//如果改变了进程，页目录也会随着变化
	if( cur->process != thr->process ){
		extern PROCESS* cur_proc;
		cur_proc = thr->process;
		load_page_dir( cur_proc->page_dir );
//		kprintf("{%d}", cur_proc->pid );
	}
	g_tss.esp0 = (uint)thr+sizeof(THREAD);
	fastcall_update_esp( g_tss.esp0 );
	//下面是汇编代码了
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
	if( thr->kernel ){
		r->gs = r->fs = r->es = r->ds = r->ss = GD_KERNEL_DATA;
		r->cs = GD_KERNEL_CODE;
	}else{
		r->gs = r->fs = r->es = r->ds = r->ss = GD_USER_DATA;
		r->cs = GD_USER_CODE;
	}
	r->eflags = 0x202;
	r->esp = stack_addr;	//一般运行时堆栈
	r->kesp = r->esp;
	r->eip = entry_addr;	//入口
	thr->stack_pointer = (t_32)r;	//中断时堆栈
}
