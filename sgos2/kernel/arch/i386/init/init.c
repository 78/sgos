//Huang Guan
//090803

#include <arch.h>
#include <multiboot.h>
#include <debug.h>

extern void kinit();	//内核初始化入口

//multiboot.S结束时调用这个函数
void multiboot_init( uint info_addr )
{
	//
	//call kinit, never return
	kinit( info_addr );
}

//进一步设置x86保护模式
#define SET_SYSTEM_GATE( vector, handle ) set_gate( vector, DA_386IGate | DA_DPL3, handle )
int machine_init()
{
	//init i386
	gdt_init();	//重新设置gdt
	isr_init();	//设置isr，捕获各种机器异常
	irq_init();	//设置irq，捕获硬、软件中断
	vm86_init();	//vm86
	//设置软中断
	SET_SYSTEM_GATE( SYSTEM_INTERRUPT, (void*)syscall_interrupt );
	//初始化fastcall
	fastcall_init();
	kprintf("Protect mode was controled successfully.\n");
	//Real time clock
	rtc_init();
	return 0;
}
