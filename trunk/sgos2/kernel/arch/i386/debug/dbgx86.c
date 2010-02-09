
#include <sgos.h>
#include <arch.h>
#include <kd.h>

//调试器中断句柄
static int dbg_handler( int err_code, I386_REGISTERS* r )
{
	//dbg can not work now
	//continue the program.
	PERROR("##debug at 0x%X is not implemented.", r->eip );
	return 1;
}

//初始化调试器
void ArInitializeDebugger()
{
	ArInstallIsr( DEBUG_INTERRUPT, (void*)dbg_handler );
}

//打印寄存器的内容
void ArDumpRegisters( const I386_REGISTERS *r )
{
	KdPrintf("pid:%d dump cpu:\ncs: 0x%X\teip: 0x%X\nss: 0x%X\tesp: 0x%X\tkesp: 0x%X\n"
		"ds: 0x%X\tesi: 0x%X\nes: 0x%X\tedi: 0x%X\nfs: 0x%X\ngs: 0x%X\neax: 0x%X\tecx: 0x%X\tebx: 0x%X\t"
		"edx: 0x%X\neflags: 0x%X\n", 0,r->cs, r->eip, r->ss, r->esp, r->kesp, 
		r->ds, r->esi, r->es, r->edi, r->fs, r->gs, r->eax, r->ecx,
		r->ebx, r->edx, r->eflags);
}


