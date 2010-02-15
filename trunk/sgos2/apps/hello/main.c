#include <sgos.h>
#include <api.h>

int ServiceEntry()
{
	Api_Print("Hello world!\n");
	Message msg = {SystemId, 0, System_ExitThread};
	Api_Send( &msg, INFINITE );
	return 0;
}
