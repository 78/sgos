//Huang Guan
//090803

#include <arch.h>
#include <multiboot.h>
#include <debug.h>

extern void kinit();

void multiboot_init( uint magic, uint addr )
{
	//
	//call kinit, no return
	kinit();
}

#define SET_SYSTEM_GATE( vector, handle ) set_gate( vector, DA_386IGate | DA_DPL3, handle )
void init_machine()
{
	//init i386
	gdt_init();
    isr_init();
    irq_init();
	SET_SYSTEM_GATE( SYSTEM_INTERRUPT, (void*)syscall_interrupt );
	kprintf("Protect mode was controled successfully.\n");
}
