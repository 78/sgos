#include <sgos.h>
#include <api.h>
#include "debug.h"

void _exit()
{
	DBG("exit()");
	SysExitThread(13);
}

