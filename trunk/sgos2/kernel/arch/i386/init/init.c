//Huang Guan
//090803

#include <arch.h>
#include <multiboot.h>
#include <debug.h>

extern void kinit();

void multiboot_init( uint info_addr )
{
	//
	//call kinit, never return
	kinit( info_addr );
}

#define SET_SYSTEM_GATE( vector, handle ) set_gate( vector, DA_386IGate | DA_DPL3, handle )
int machine_init()
{
	//init i386
	gdt_init();
	isr_init();
	irq_init();
	SET_SYSTEM_GATE( SYSTEM_INTERRUPT, (void*)syscall_interrupt );
	kprintf("Protect mode was controled successfully.\n");
	rtc_init();
	return 0;
}
