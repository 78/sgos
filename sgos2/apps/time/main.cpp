#include <stdio.h>
#include <string.h>
#include <sgos.h>
#include <api.h>
#include "biostime.h"

void parse( Message& msg )
{
	int ret;
	switch( msg.Command ){
	default:
		printf("[time]Unknown command: %x\n", msg.Command );
		msg.Code = -ERR_WRONGARG;
	}
	ReplyMessage( &msg );
}

int startService()
{
	//接收消息
	Message msg;
	int id;
	//初始化驱动程序
	if( InitializeBiosTime() < 0 )
		SysExitSpace((uint)-1);
	//注册线程名称，其它程序便可以向此服务线程发送消息
	id = SmNotifyService( 0, 0, "Time" );
	if( id < 0 ){
		printf("[time]add service failed.\n");
		SysExitSpace((uint)-1);
	}
	printf("[Time]Starting Time service ...\n");
	for(;;){
		//Pending for messages
		int result = WaitMessage(&msg);
		if( result < 0 ){
			printf("[Time]Failed to receive message: result = %d\n", result );
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

