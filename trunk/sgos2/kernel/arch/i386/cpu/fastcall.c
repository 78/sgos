#include <sgos.h>
#include <arch.h>
#include <kd.h>

//使用sysenter需要gdt按照一定的要求设计。
//sgos2的gdt符合这个要求。
//初始化fastcall要使用到的寄存器
extern void ArAsmSysenterHandler();
void ArInitializeFastcall()
{
	wrmsr(MSR_IA32_SYSENTER_CS, GD_KERNEL_CODE, 0);
	wrmsr(MSR_IA32_SYSENTER_EIP, (size_t)&ArAsmSysenterHandler, 0);

}

//更新fastcall要使用的线程内核堆栈
void ArUpdateFastcallEsp(size_t kesp)
{
	wrmsr(MSR_IA32_SYSENTER_ESP, kesp, 0);
}

