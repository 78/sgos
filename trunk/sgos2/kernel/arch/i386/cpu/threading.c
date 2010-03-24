#include <sgos.h>
#include <arch.h>
#include <kd.h>
#include <tm.h>
#include <mm.h>
#include <rtl.h>

static TSS g_tss;

#define GET_THREAD_REGS(p) (I386_REGISTERS*)((t_32)p+sizeof(KThread)-sizeof(I386_REGISTERS) )

void ArAsmEnterThreadingMode(size_t stackptr);
void KeResumeStart();
void ArStartThreading()
{
	//TSS, 保存着内核所用的堆栈, 从特权3到特权0要借助TSS
	RtlZeroMemory(&g_tss, sizeof(TSS) );
	g_tss.ss0 = GD_KERNEL_DATA;
	g_tss.esp0 = 0;
	g_tss.iobase = (t_32)g_tss.iomap - (size_t)&g_tss;//sizeof(TSS);
	//设置TSS描述符
	ArSetGdtEntry( GD_TSS_INDEX, (size_t)&g_tss, sizeof(TSS)-1, DA_386TSS );
	//加载TSS和LDT
	__asm__ __volatile__("mov $0x28, %bx; ltr %bx");
	//进入线程模式
	ArAsmEnterThreadingMode((size_t)(TmGetCurrentThread())+sizeof(KThread));
	//开启中断，引发线程切换
	ArLocalEnableIrq();
	//继续初始化。
	KeResumeStart();
}

// 设置下一个运行的线程环境
void ArPrepareForNextThread()
{
	KThread* next = ThreadingBox.next;
	if( !next )	
		return;
	if( !next->IsKernelThread ){
		//更新中断的线程内核栈
		g_tss.esp0 = (uint)next+sizeof(KThread);
		//更新fastcall使用的线程内核栈
		ArUpdateFastcallEsp( g_tss.esp0 );
		//设置用户态线程信息段，fs使用
		ArSetGdtEntry( GD_TIB_INDEX, (t_32)next->UserModeThreadInformation, 
			PAGE_ALIGN(sizeof(ThreadInformation))-1, DA_DRW | DA_32 | DA_DPL3 );
	}
	//如果改变了地址空间，则更新页目录
	if( MmGetCurrentSpace() != next->Space )
		ArLoadPageDirectory( &next->Space->PageDirectory );
	//检查数学协处理器
	ArCheckAndSaveFpu( ThreadingBox.running );
}

//线程切换
void ArSwitchThread( KThread* cur, KThread* next )
{
	//改变环境
	ArPrepareForNextThread();
	//改变当前线程
	ThreadingBox.running = next;
	ThreadingBox.next = NULL;
	//下面是汇编代码了
	ArAsmSwapContext( NULL, &cur->StackPointer, &next->StackPointer );
}

//初始化线程的寄存器信息。
void ArInitializeThreadRegisters( KThread* thr, KThread* parent,
	void* context, size_t entry_addr, size_t stack_addr )
{
	I386_REGISTERS *r;
	if( !parent )
		return;
	//获取堆栈的寄存器帧，通过设置这里的数据改变返回地址和返回后的寄存器
	r = GET_THREAD_REGS(thr);	
	if( thr->InBiosMode ){	//VM86 线程
		uint far_ptr;
		ArchThread* arch;
		arch = &thr->ArchitectureInformation;
		arch->vmflag_if = 1; // allow interrupts
		arch->in_vm86 = 1; //toggle vm86 mode.
		far_ptr = LINEAR_TO_FARPTR( entry_addr );
		r->cs = r->ss = FARPTR_SEG(far_ptr);
		r->eip = FARPTR_OFF(far_ptr);
		r->esp = 0xFFE0;	//the end of 64KB
		r->eflags = 0x202|EFLAG_VM|EFLAG_IF;	//VM|IF
		thr->StackPointer = (t_32)r;	//中断时堆栈
	}else{	//32位保护模式
		//判断是否内核态
		if( thr->IsKernelThread ){
			r->es = r->ds = r->ss = GD_KERNEL_DATA;
			r->cs = GD_KERNEL_CODE;
		}else{
			//用户态段寄存器
			r->es = r->ds = r->ss = GD_USER_DATA;
			r->fs = GD_USER_TIB;
			r->cs = GD_USER_CODE;
		}
		r->eflags = 0x202;
		if( thr->Space->UserId == ADMIN_USER ){
			//允许系统线程使用IO
			//r->eflags |= EFLAG_IOPL3;
		}
		r->esp = stack_addr;	//一般运行时堆栈
		r->eip = entry_addr;	//入口
		thr->StackPointer = (t_32)r;	//中断时堆栈
	}
}

//释放线程使用的资源，例如fpu
void ArReleaseThreadResources( KThread* thr )
{
	if(thr->ArchitectureInformation.fsave){
		MmFreeKernelMemory(thr->ArchitectureInformation.fsave);
		thr->ArchitectureInformation.fsave = NULL;
		thr->UsedMathProcessor = 0;
	}
}
