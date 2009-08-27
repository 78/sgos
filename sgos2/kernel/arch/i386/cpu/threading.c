#include <sgos.h>
#include <arch.h>
#include <debug.h>
#include <thread.h>
#include <process.h>
#include <string.h>
#include <mm.h>

static TSS g_tss;

#define GET_THREAD_REGS(p) (I386_REGISTERS*)((t_32)p+sizeof(THREAD)-sizeof(I386_REGISTERS) )

void enter_threading_mode(size_t stackptr);
void kinit_resume();
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
	//进入线程模式
	enter_threading_mode((size_t)(current_thread())+sizeof(THREAD));
	//开启中断，引发线程切换
	local_irq_enable();
	//继续初始化。
	kinit_resume();
}

// 设置下一个运行的线程环境
void update_for_next_thread()
{
	THREAD* next = tbox.next;
	if( !next )	
		return;
	if( !next->kernel ){
		//更新中断的线程内核栈
		g_tss.esp0 = (uint)next+sizeof(THREAD);
		//更新fastcall使用的线程内核栈
		fastcall_update_esp( g_tss.esp0 );
		//设置用户态线程信息段，fs使用
		set_gdt_desc( GD_TIB_INDEX, (t_32)next->thread_info, 
			PAGE_ALIGN(sizeof(THREAD_INFO))-1, DA_DRW | DA_32 | DA_DPL3 );
	}
	//如果改变了进程，页目录也会随着变化
	if( current_proc() != next->process )
		load_page_dir( next->process->page_dir );
	
}

//线程切换
void switch_to( THREAD* cur, THREAD* next )
{
	//改变环境
	update_for_next_thread();
	//改变当前线程
	tbox.running = next;
	tbox.next = NULL;
	//下面是汇编代码了
	i386_switch( NULL, &cur->stack_pointer, &next->stack_pointer );
}

//初始化线程的寄存器信息。
void init_thread_regs( THREAD* thr, THREAD* parent,
	void* context, uint entry_addr, uint stack_addr )
{
	PROCESS* proc;
	I386_REGISTERS *r;
	if( !parent )
		return;
	proc = thr->process;
	//获取堆栈的寄存器帧，通过设置这里的数据改变返回地址和返回后的寄存器
	r = GET_THREAD_REGS(thr);	
	if( proc->bios_mode ){	//VM86...
		uint far_ptr;
		ARCH_THREAD* arch;
		arch = &thr->arch;
		arch->vmflag_if = 1; // allow interrupts
		arch->in_vm86 = 1; //toggle vm86 mode.
		far_ptr = LINEAR_TO_FARPTR( entry_addr );
		r->cs = r->ss = FARPTR_SEG(far_ptr);
		r->eip = FARPTR_OFF(far_ptr);
		r->esp = 0xFFFF;	//the end of 64KB
		r->eflags = 0x202|EFLAG_VM|EFLAG_IF;	//VM|IF
		thr->stack_pointer = (t_32)r;	//中断时堆栈
	}else{	//32位保护模式
		//判断是否内核态
		if( thr->kernel ){
			r->es = r->ds = r->ss = GD_KERNEL_DATA;
			r->cs = GD_KERNEL_CODE;
		}else{
			//用户态段寄存器
			r->es = r->ds = r->ss = GD_USER_DATA;
			r->fs = GD_USER_TIB;
			r->cs = GD_USER_CODE;
		}
		r->eflags = 0x202;	//标志寄存器
		if( thr->process->uid == ADMIN_USER ){
			//允许系统进程使用IO
			r->eflags |= EFLAG_IOPL3;
		}
		r->esp = stack_addr;	//一般运行时堆栈
		r->eip = entry_addr;	//入口
		thr->stack_pointer = (t_32)r;	//中断时堆栈
	}
}
