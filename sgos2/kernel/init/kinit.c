#include <sgos.h>
#include <debug.h>

void kinit()
{
	debug_init();
	KERROR("##Warning: kernel not ready.");
}

