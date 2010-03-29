#include <sgos.h>
#include <api.h>
#include "debug.h"

int kill(int pid,int sig)
{
	return PsTerminateProcess( pid, sig );
}
