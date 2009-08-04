//Huang Guan
//090803

#include <sgos.h>
#include <multiboot.h>
#include <debug.h>

extern void kinit();

void multiboot_init( uint magic, uint addr )
{
	//init i386
	//
	//call kinit, no return
	kinit();
}

