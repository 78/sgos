#include <sgos.h>
#include <api.h>
#include <stdlib.h>


void sleep(unsigned int ms)
{
	sys_thread_wait(ms);
}
