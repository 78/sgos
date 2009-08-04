#include <sgos.h>
#include <debug.h>

void kinit()
{
	debug_init();
	init_machine();
	int i=0;
	i = 4/i;
	KERROR("##Warning: kernel not ready.");
}

