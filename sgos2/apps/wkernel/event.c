#include <sgos.h>
#include <api.h>
#include "debug.h"


void WINAPI OpenEventA()
{
	NOT_IMPLEMENTED();
}


void WINAPI CreateEventA()
{
	NOT_IMPLEMENTED();
}


int WINAPI SetEvent(uint h)
{
	if( h==0 )
		return 0;
	NOT_IMPLEMENTED();
}


void WINAPI ResetEvent()
{
	NOT_IMPLEMENTED();
}


void WINAPI PulseEvent()
{
	NOT_IMPLEMENTED();
}
