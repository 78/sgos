#include <sgos.h>
#include <api.h>

void ParseCommand(Message& msg )
{
	char* bufferPage = (char*)SysAllocateMemory( SysGetCurrentSpaceId(), 
		BUFFER_PAGES_SIZE, 0, ALLOC_SWAP );
	switch(msg.Command){
	case Pe_Load:
		SysSwapMemory( SPACEID(msg.ThreadId), msg.Large[0], (size_t)bufferPages, PAGE_SIZE, MAP_ADDRESS );
		msg.Code = PeLoadLibrary( msg.Argumemts[0], msg.Large[0], msg.Arguments[1] );
		break;
	case Pe_Unload:
		msg.Code = PeUnloadLibrary( msg.Arguments[0], msg.Arguments[1] );
		break;
	case Pe_GetProcedureAddress:
		SysSwapMemory( SPACEID(msg.ThreadId), msg.Large[0], (size_t)bufferPages, PAGE_SIZE, MAP_ADDRESS );
		msg.Code = PeGetProcedureAddress( msg.Arguments[0], msg.Arguments[1], bufferPages );
		break;
	}
}

void DoMessageLoop()
{
	Message msg;
	int result;
	for(;;){
		result = WaitMessage( &msg );
		if( result < 0 ) {
			printf("[pe]receive message failed. result=%d\n", result);
			continue;
		}
		ParseCommand(msg);
		ReplyMessage(&msg);
	}
}

int main()
{
	int sid = SmNoitfyService( PeLoaderId, 0, "PeLoader" );
	DoMessageLoop();
	SmRemoveService( sid );
}
