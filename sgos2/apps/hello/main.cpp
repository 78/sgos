#include <sgos.h>
#include <api.h>
#include <stdio.h>

void beep()
{
	Message msg;
	int ret;
	uint tid = SmGetServiceThreadByName("Speaker");
	if( tid == 0 ){
		printf("[hello] Speaker service not found.\n");
		return;
	}
	msg.ThreadId = tid;
	msg.Command = 3;
	ret = Api_Send(&msg, 0);
	if( ret < 0 ){
		printf("[hello] Failed to send command to service.\n");
		return ;
	}
	Api_Receive(&msg, 3000);
	//Omit the result.
}

int main()
{
	printf("[hello]Program started.\n");
	//Wait for Speaker Service to start.
	SysSleepThread(1000);
	beep();
	return 0;
}
