#include <stdio.h>
#include <string.h>
#include <sgos.h>
#include <api.h>
#include "acpi.h"

void parse( Message& msg )
{
	int ret;
	switch( msg.Command ){
	case Acpi_PowerOff:
		AcpiPowerOff();
		break;
	default:
		printf("[Acpi]Unknown command: %x\n", msg.Command );
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
	if( InitializeAcpi() < 0 )
		SysExitSpace((uint)-1);
	//注册线程名称，其它程序便可以向此服务线程发送消息
	id = SmNotifyService( 0, 0, "Acpi" );
	if( id < 0 ){
		printf("[Acpi]add service failed.\n");
		SysExitSpace((uint)-1);
	}
	printf("[Acpi]Starting Acpi service ...\n");
	for(;;){
		//Pending for messages
		int result = WaitMessage(&msg);
		if( result < 0 ){
			printf("[Acpi]Failed to receive message: result = %d\n", result );
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

