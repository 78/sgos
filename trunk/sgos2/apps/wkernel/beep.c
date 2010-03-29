#include <sgos.h>
#include <api.h>

void WINAPI Beep(uint freq, uint duration)
{
	Message msg;
	int ret;
	uint tid = SmGetServiceThreadByName("Speaker");
	if( tid == 0 ){
		printf("[wkernel] Speaker service not found.\n");
		return;
	}
	msg.ThreadId = tid;
	msg.Command = 2;
	msg.Arguments[0] = freq;
	msg.Arguments[1] = duration;
	ret = Api_Send(&msg, 0);
	if( ret < 0 ){
		printf("[wkernel] Failed to send command to service.\n");
		return ;
	}
	Api_Receive(&msg, 3000);
	//Omit the result.
}

