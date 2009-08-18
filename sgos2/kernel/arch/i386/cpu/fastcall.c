#include <sgos.h>
#include <arch.h>
#include <string.h>
#include <debug.h>

//使用sysenter需要gdt按照一定的要求设计。
//sgos2的gdt符合这个要求。

extern void sysenter_handler();
void fastcall_init()
{
	wrmsr(MSR_IA32_SYSENTER_CS, GD_KERNEL_CODE, 0);
	wrmsr(MSR_IA32_SYSENTER_EIP, (uint)&sysenter_handler, 0);

}

void fastcall_update_esp(uint kesp)
{
	wrmsr(MSR_IA32_SYSENTER_ESP, kesp, 0);
}

