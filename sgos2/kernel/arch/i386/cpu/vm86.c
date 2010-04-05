// 090826 Written by Xiaoxia.
// Please help fix up some problems.

#include <sgos.h>
#include <arch.h>
#include <tm.h>
#include <mm.h>
#include <kd.h>
#include <ipc.h>

static KSemaphore sem_bioscall;

//线性地址转换为实模式地址
size_t LINEAR_TO_FARPTR(size_t ptr)
{
    unsigned seg, off;
    off = ptr & 0xffff;
    seg = (ptr & 0xffff0000) >> 4;
    return MAKE_FARPTR(seg, off);
}

// VM86 General Protection Fault ...
#define VALID_FLAGS	0xDFF
static int gpf_handler( int err_code, I386_REGISTERS* r)
{
	t_8* ip;
	t_16* stack, *ivt, *stack32;
	KThread* thr;
	ArchThread* arch;
	int is_data32, is_addr32;
	thr = TmGetCurrentThread();
	// X86相关线程信息
	arch = &thr->ArchitectureInformation;
	// 非vm86模式下的异常不进行处理
	if( !arch->in_vm86 )
		return 0;
	// 得到ip的线性地址
	ip = (t_8*)FARPTR_TO_LINEAR(r->cs, r->eip);
	// 得到堆栈的线性地址
	stack = (t_16*)FARPTR_TO_LINEAR(r->ss, r->esp);
	stack32 = stack;
	// 默认的bios中断向量在地址0处
	ivt = (t_16*)0;
	// 是否32位操作
	is_data32 = is_addr32 = 0;
	for(;;){
		// Check the Opcode
		switch( ip[0] ){
		case 0x66:	//32bit data
			is_data32 = 1;
			ip ++;
			r->eip = (t_16) (r->eip + 1);
			break;
		case 0x67:	//32bit address
			is_addr32 = 1;
			ip ++;
			r->eip = (t_16) (r->eip + 1);
			PERROR("##not supported not.");
			break;
		case 0xCD:	//int n
			switch( ip[1] ){
			case 0xFB:
			case 0x03: //breakpoint
				TmTerminateThread( thr, r->eax );
				return 0;
			case 0xFC: //delay 5 seconds
				TmSleepThread( thr, r->eax );
				r->eip = (t_16) (r->eip + 1);
				return 1;
			default:
//				KdPrintf("vm86 interrupt 0x%x at 0x%X\n", ip[1], r->eip );
				stack -= 3;
				r->esp = ((r->esp & 0xffff) - 6) & 0xffff;
				stack[0] = (t_16) (r->eip + 2);
				stack[1] = r->cs;
				stack[2] = (t_16) r->eflags;
				//!!check the IF if necessary
				if( arch->vmflag_if )
					stack[2] |= EFLAG_IF;
				else
					stack[2] &= ~EFLAG_IF;
				r->cs = ivt[ip[1] * 2 + 1];
				r->eip = ivt[ip[1] * 2];
				return 1;
			}
		case 0x9c:	//pushf
			if( is_data32 ){
				r->esp = ((r->esp & 0xffff) - 4) & 0xffff;
				stack32--;
				stack32[0] = r->eflags & VALID_FLAGS;
				if( arch->vmflag_if )
					stack32[0] |= EFLAG_IF;
				else
					stack32[0] &= ~EFLAG_IF;
			}else{
				r->esp = ((r->esp & 0xffff) - 2) & 0xffff;
				stack--;
				stack[0] = (t_16) r->eflags;
				if( arch->vmflag_if )
					stack[0] |= EFLAG_IF;
				else
					stack[0] &= ~EFLAG_IF;
			}
			r->eip = (t_16) (r->eip + 1);
			return 1;
		case 0x9d:	//popf
			if( is_data32 ){
				r->eflags = (stack32[0] & VALID_FLAGS) |EFLAG_VM|EFLAG_IF;
				r->esp = ((r->esp & 0xffff) + 4) & 0xffff;
				arch->vmflag_if = (stack32[0]&EFLAG_IF)!=0;
			}else{
				r->eflags = stack[0]|EFLAG_VM|EFLAG_IF;
				r->esp = ((r->esp & 0xffff) + 2) & 0xffff;
				arch->vmflag_if = (stack[0]&EFLAG_IF)!=0;
			}
			r->eip = (t_16) (r->eip + 1);
			return 1;
		case 0xCF:	//iret
			if( is_data32 ){
				r->eip = stack32[0];
				r->cs = stack32[1];
				r->eflags = EFLAG_VM|EFLAG_IF|stack32[2];
				arch->vmflag_if = (stack32[2]&EFLAG_IF)!=0;
				r->esp = ((r->esp & 0xffff) + 12) & 0xffff;
			}else{
				r->eip = stack[0];
				r->cs = stack[1];
				r->eflags = EFLAG_VM|EFLAG_IF|stack[2];
				arch->vmflag_if = (stack[2]&EFLAG_IF)!=0;
				r->esp = ((r->esp & 0xffff) + 6) & 0xffff;
			}
			return 1;
		case 0xFA:	//cli
			arch->vmflag_if = 0;
			r->eip = (t_16) (r->eip + 1);
			return 1;
		case 0xFB:	//sti
			arch->vmflag_if = 1;
			r->eip = (t_16) (r->eip + 1);
			return 1;
		default:
			PERROR("Unhandled Opcode: 0x%X at 0x%X", ip[0], ip );
			return 0;
		}
	}
	return 0; //kill the thread
}

//VM86初始化
void ArInitializeVm86()
{
	// 对运行在vm86下的异常进行处理
	ArInstallIsr( GPF_INTERRUPT, (void*)gpf_handler );
	// 此时应该可以初始化信号灯吧
	IpcInitializeSemaphore( &sem_bioscall );
}

//BIOS调用
/* 最简单的方法是创建一个VM86线程，然后跳转过去调用bios */
int ArInvokeBiosService( int interrupt, void* context_ptr, size_t siz )
{
	KThread* thr;
	I386_CONTEXT* context;
	t_16* stack;
	int i;
	if( siz < sizeof(I386_CONTEXT) )
		return -ERR_WRONGARG;
	// 映射物理内存前1MB
	ArMapMultiplePages( &MmGetCurrentSpace()->PageDirectory, 0, 
		0, MB(1), PAGE_ATTR_PRESENT|PAGE_ATTR_WRITE|PAGE_ATTR_USER, MAP_ADDRESS|MAP_ATTRIBUTE );
	
	context = (I386_CONTEXT*)context_ptr;
	stack = (t_16*)0x0001FFE0;
	// 因为bios调用不支持多线程，因此同一时刻只能一个线程使用。
	IpcLockSemaphore( &sem_bioscall );
	// 设置堆栈
	i = 0;
	// 获取寄存器信息
	stack[i++] = (t_8)interrupt;
	stack[i++] = context->es;
	stack[i++] = context->ds;
	stack[i++] = context->edi;
	stack[i++] = context->esi;
	stack[i++] = context->ebp;
	stack[i++] = context->kesp;
	stack[i++] = context->ebx;
	stack[i++] = context->edx;
	stack[i++] = context->ecx;
	stack[i++] = context->eax;
	// 创建VM86线程
	thr = TmCreateThread( MmGetCurrentSpace(), 0x10100, BIOS_THREAD );
	if( thr==NULL ){
		IpcUnlockSemaphore( &sem_bioscall );
		return -ERR_UNKNOWN;
	}
	TmResumeThread( thr );
	// 等待线程结束
	TmJoinThread( thr );
	// 保存寄存器信息
	i = 1;
	context->es = stack[i++];
	context->ds = stack[i++];
	context->edi = stack[i++];
	context->esi = stack[i++];
	context->ebp = stack[i++];
	context->kesp = stack[i++];
	context->ebx = stack[i++];
	context->edx = stack[i++];
	context->ecx = stack[i++];
	context->eax = stack[i++];
	//
	IpcUnlockSemaphore( &sem_bioscall );
	ArUnmapMultiplePages( &MmGetCurrentSpace()->PageDirectory, 0, MB(1) );
	return 0;
}

