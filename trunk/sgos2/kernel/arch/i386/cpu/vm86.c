// 090826 Written by Huang Guan.
// Please help fix up some problems.

#include <sgos.h>
#include <arch.h>
#include <thread.h>
#include <process.h>
#include <mm.h>
#include <debug.h>

uint LINEAR_TO_FARPTR(size_t ptr)
{
    unsigned seg, off;
    off = ptr & 0xf;
    seg = (ptr - (ptr & 0xf)) >> 4;
    return MAKE_FARPTR(seg, off);
}

// VM86 General Protection Fault ...
int gpf_handler( int err_code, I386_REGISTERS* r)
{
	t_8* ip;
	t_16* stack, *ivt, *stack32;
	THREAD* thr;
	ARCH_THREAD* arch;
	int is_data32, is_addr32;
	thr = current_thread();
	// X86相关线程信息
	arch = &thr->arch;
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
			case 0x03: //debug
				PERROR("[%d]VM86 thread exit with code:0x%X", thr->tid, r->eax);
				thread_terminate( thr, r->eax );
				return 0;
				//never return here
			case 0xFC: //delay 5 seconds
				thread_wait(r->eax);
				r->eip = (t_16) (r->eip + 1);
				return 1;
			default:
				kprintf("vm86 interrupt 0x%x at 0x%X\n", ip[1], r->eip );
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
				stack32[0] = r->eflags;
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
				r->eflags = stack32[0]|EFLAG_VM|EFLAG_IF;
				r->esp = ((r->esp & 0xffff) + 4) & 0xffff;
				arch->vmflag_if = (stack32[0]&EFLAG_IF)!=0;
			}else{
				r->eflags = stack[0]|EFLAG_VM|EFLAG_IF;
				r->esp = ((r->esp & 0xffff) + 2) & 0xffff;
				arch->vmflag_if = (stack[0]&EFLAG_IF)!=0;
			}
			r->eip = (t_16) (r->eip + 1);
			return 1;
		case 0xEE:	//out byte
			out_byte(r->edx, r->eax&0xFF);
			r->eip = (t_16) (r->eip + 1);
			return 1;
		case 0xEF:	//out word
			/* 我看的资料上竟然没有说在0xEF前加0x66要输出32位值，
			 * 在VMware上无法改变分辨率。后来发现此处最可疑，想不到
			 * 改成dword后竟然成功了。我真聪明:-)
			 */
			if( is_data32 ){
				out_dword(r->edx, r->eax);
			}else{
				out_word(r->edx, r->eax&0xFFFF);
			}
			r->eip = (t_16) (r->eip + 1);
			return 1;
		case 0xEC:	//in byte
			r->eax = (r->eax&0xFFFFFF00)|in_byte(r->edx);
			r->eip = (t_16) (r->eip + 1);
			return 1;
		case 0xED:	//in word
			if( is_data32 ){
				r->eax = in_dword(r->edx);
			}else{
				r->eax = (r->eax&0xFFFF0000)|in_word(r->edx);
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
			kprintf("eax=0x%X\n", r->eax );
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

void vm86_init()
{
	// 对运行在vm86下的异常进行处理
	isr_install( GPF_INTERRUPT, (void*)gpf_handler );
}

