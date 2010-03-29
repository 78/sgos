#include <sgos.h>
#include <api.h>
#include "debug.h"


void WINAPI QueryPerformanceCounter()
{
	NOT_IMPLEMENTED();
}


int WINAPI QueryPerformanceFrequency(int* freq)
{
	return 0; //failed.
}
