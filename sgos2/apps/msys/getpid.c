#include <sgos.h>
#include <api.h>
#include "debug.h"

int getpid()
{
	return PsGetCurrentProcessId();
}
