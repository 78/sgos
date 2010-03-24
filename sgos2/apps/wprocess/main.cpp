#include <sgos.h>
#include <api.h>
#include <stdio.h>
#include <string.h>
#include "module.h"

extern void ProcessTest();
static void ServiceMessageLoop()
{
	Message msg = {0};
	int result;
	printf("[wprocess]Program started.\n");
	ProcessTest();
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
				break;
			}
			case wProcess_Terminate:
			{
				break;
			}
			case wProcess_Suspend:
			{
				break;
			}
			case wProcess_Resume:
			{
				break;
			}
		}
		ReplyMessage( &msg );
	}
}


int main()
{
	printf("[wprocess]SpaceId: %d\n", SysGetCurrentSpaceId() );
	//Add Me
	if( SmNotifyService( 0, 0, "wProcess" ) < 0 )
		printf("[wprocess]Failed to add service.\n");
	//Do Service Loop
	ServiceMessageLoop();
	return 0;
}
