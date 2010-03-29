#include <sgos.h>
#include <api.h>
#include "debug.h"


void WINAPI SetEnvironmentVariableA()
{
	NOT_IMPLEMENTED();
}


void WINAPI GetEnvironmentStrings()
{
	NOT_IMPLEMENTED();
}


size_t WINAPI GetEnvironmentVariableA(const char* name, char* buf, size_t buf_len)
{
	DBG("name: %s  buf_len:0x%x", name, buf_len );
	return 0;
}


void WINAPI FreeEnvironmentStringsA()
{
	NOT_IMPLEMENTED();
}


void WINAPI GetCommandLineA()
{
	NOT_IMPLEMENTED();
}


void WINAPI ExpandEnvironmentStringsA()
{
	NOT_IMPLEMENTED();
}
