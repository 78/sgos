#include <sgos.h>
#include <api.h>
#include <stdio.h>
#include <string.h>
#include "module.h"
#include "wprocess.h"

extern void ProcessTest();
static void ServiceMessageLoop()
{
	Message msg = {0};
	int result;
	printf("[wprocess]Service started.\n");
	for( ;; ){
		result = WaitMessage( &msg );
		if( result < 0 ){
			printf("[wprocess] Failed in Api_Receive: result = %d\n", result );
			continue;
		}
		msg.Code = 0; //set it to success
		switch( msg.Command ){
			case wProcess_Create:
			{
				char* cmdline = (char*)SysAllocateMemory( SysGetCurrentSpaceId(), PAGE_SIZE, MEMORY_ATTR_WRITE, ALLOC_SWAP );
				char* env = (char*)SysAllocateMemory( SysGetCurrentSpaceId(), ENVIRONMENT_STRING_SIZE, MEMORY_ATTR_WRITE, ALLOC_SWAP );
				SysSwapMemory( SPACEID(msg.ThreadId), (size_t)msg.Large[0], (size_t)cmdline, PAGE_SIZE, MAP_ADDRESS );
				SysSwapMemory( SPACEID(msg.ThreadId), (size_t)msg.Large[1], (size_t)env, ENVIRONMENT_STRING_SIZE, MAP_ADDRESS );
				Process* ps = new Process( msg.Arguments[0], cmdline, env );
				if( !ps )
					msg.Code = -ERR_WRONGARG;
				break;
			}
			case wProcess_Terminate:
			{
				Process* ps = Process::GetProcessById( msg.Arguments[0] );
				if( ps )
					ps->Terminate( msg.Arguments[1] );
				break;
			}
			case wProcess_Suspend:
			{
				Process* ps = Process::GetProcessById( msg.Arguments[0] );
				if( ps )
					ps->Suspend();
				break;
			}
			case wProcess_Resume:
			{
				Process* ps = Process::GetProcessById( msg.Arguments[0] );
				if( ps )
					ps->Resume();
				break;
			}
			case wProcess_CreateThread:
			{
				Process* ps = Process::GetProcessById( SPACEID( msg.ThreadId ) );
				if( !ps ){
					msg.Code = -ERR_WRONGARG;
					break;
				}
				Thread* ts = ps->CreateThread( msg.Arguments[0] );
				if( !ts ){
					msg.Code = -ERR_NOMEM;
					break;
				}
				msg.Code = ts->ThreadId();
				break;
			}
			case wProcess_TerminateThread:
			{
				Process* ps = Process::GetProcessById( SPACEID( msg.ThreadId ) );
				if( !ps ){
					msg.Code = -ERR_WRONGARG;
					break;
				}
				Thread* t = ps->GetThreadById( msg.Arguments[0] );
				if( t )
					t->Terminate( msg.Arguments[1] );
				break;
			}
		}
		ReplyMessage( &msg );
	}
}

void CreateStartupProcess()
{
	for(;;){
		char* cmdline = (char*)SysAllocateMemory( SysGetCurrentSpaceId(), PAGE_SIZE, MEMORY_ATTR_WRITE, 0 );
		strcpy( cmdline, "/c:/sgos/startup.exe" );
		Process *p=0;
		p = new Process( 0, cmdline, 0 );
		if( p->ProcessId()>0 )
			break;
		delete p;
		SysSleepThread(200);
	}
}

int main()
{
	CreateStartupProcess();
	//Add Me
	if( SmNotifyService( wProcessId, 0, "wProcess" ) < 0 )
		printf("[wprocess]Failed to add service.\n");
	//Do Service Loop
	ServiceMessageLoop();
	return 0;
}
