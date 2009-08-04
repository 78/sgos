#include <sgos.h>
#include <debug.h>

void kinit()
{
	debug_init();
	init_machine();
	KERROR("##Warning: kernel not ready.");
}

