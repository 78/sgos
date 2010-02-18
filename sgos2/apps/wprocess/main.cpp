#include <sgos.h>
#include <api.h>
#include <stdio.h>
#include <string.h>


static void ServiceMessageLoop()
{
	Message msg = {0};
	int result;
	printf("[wprocess]Program started.\n");
	for( ;; ){
		result = ReceiveMessage( &msg, INFINITE );
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
	printf("SpaceId: %d\n", GetCurrentSpaceId() );
	//Add Me
	NotifyService( 0, 0, "wProcess" );
	//Do Service Loop
	ServiceMessageLoop();
	return 0;
}
