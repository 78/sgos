#include <stdio.h>
#include <string.h>
#include <sgos.h>
#include <api.h>
#include "vesa.h"

void parse( Message& msg )
{
	int ret;
	switch( msg.Command ){
	case Video_SetGraphicalMode:
		msg.Code = SetGraphicalMode();
		break;
	case Video_SetCharacterMode:
		msg.Code = SetCharacterMode();
		break;
	case Video_SetVideoMode:
		msg.Code = SetVideoMode( msg.Arguments[0], msg.Arguments[1], msg.Arguments[2] );
		break;
	case Video_GetVideoMode:
		msg.Code = GetVideoMode( msg.Arguments[0], msg.Arguments[1], msg.Arguments[2] );
		break;
	case Video_GetVideoAddress:
		msg.Code = GetVideoAddress( msg.Arguments[0], msg.Arguments[1] );
		break;
	default:
		printf("[Vesa]Unknown command: %x\n", msg.Command );
		msg.Code = -ERR_WRONGARG;
	}
	ReplyMessage( &msg );
}

int startService()
{
	//接收消息
	Message msg;
	int id;
	//注册线程名称，其它程序便可以向此服务线程发送消息
	id = SmNotifyService( 0, 0, "Vesa" );
	if( id < 0 ){
		printf("[Vesa]add service failed.\n");
		SysExitSpace((uint)-1);
	}
	//初始化驱动程序
	InitializeVesa();
	printf("[Vesa]Starting Vesa service ...\n");
	for(;;){
		//Pending for messages
		int result = WaitMessage(&msg);
		if( result < 0 ){
			printf("[Vesa]Failed to receive message: result = %d\n", result );
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

