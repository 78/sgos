#include <stdio.h>
#include <string.h>
#include <sgos.h>
#include <api.h>
#include "keyboard.h"

void parse( Message& msg )
{
	int ret;
	switch( msg.Command ){
	case System_Interrupt:
		HandleKeyboardInterrupt( msg.Arguments[0] );
		break;
	case Keyboard_SetHandler:
		if( msg.Arguments[0]==1 )
			AddHandler( msg.ThreadId, msg.Arguments[1], msg.Arguments[2] );
		else
			DelHandler( msg.ThreadId );
		break;
	case Keyboard_Configure:
		printf("[Keyboard] Configuration is not implemented.\n");
		break;
	default:
		printf("[Keyboard]Unknown command: %x\n", msg.Command );
		msg.Code = -ERR_WRONGARG;
	}
}

int startService()
{
	//接收消息
	Message msg;
	int id;
	//注册线程名称，其它程序便可以向此服务线程发送消息
	id = SmNotifyService( 0, 0, "Keyboard" );
	if( id < 0 ){
		printf("[Keyboard]add service failed.\n");
		SysExitSpace((uint)-1);
	}
	//初始化驱动程序
	InitializeKeyboard();
	printf("[Keyboard]Starting Keyboard service ...\n");
	for(;;){
		//Pending for messages
		int result = WaitMessage(&msg);
		if( result < 0 ){
			printf("[Keyboard]Failed to receive message: result = %d\n", result );
			continue;
		}
		msg.Code = 0;
		parse( msg );
	}
	SmRemoveService( id );
	return 0;
}

int main()
{
	return startService();
}

