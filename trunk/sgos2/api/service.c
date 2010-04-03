#include <sgos.h>
#include <stdio.h>
#include <api.h>


static ServiceInformation* siList = (ServiceInformation*)0; 

static void GetServiceListPtr()
{
	SystemInformation* si = (SystemInformation*)SysGetSystemInformation();
	if( !si ){
		printf("[Service]Failed to get system information.\n");
		return;
	}
	siList = si->ServiceList;
}

int SmNotifyService( uint serviceId, uint eventFlag, const char* name )
{
	uint tid = SmGetServiceThreadByName( "ServiceManager" );
	if( !tid ){
		printf("Failed to get ServiceManager thread.\n");
		return -ERR_UNKNOWN;
	}
	Message msg = { tid, 0, Service_Notify};
	int result;
	msg.Arguments[0] = serviceId;
	msg.Arguments[1] = eventFlag;
	msg.Arguments[2] = SysGetCurrentThreadId();
	strcpy( (char*)&msg.Arguments[3], name );
	Api_Send( &msg, 0 );
	result = Api_Receive( &msg, 3000 );
	if( result == -ERR_TIMEOUT )
		printf("## Error: ServiceManager no reply\n");
	if( result < 0 )
		return result;
	return msg.Code;
}

int SmRemoveService( uint serviceId )
{
	int tid = SmGetServiceThreadById( serviceId );
	int ret = 0;
	SendMessage( tid, Service_Remove, &serviceId, NULL, NULL, NULL, &ret );
	return ret;
}

uint SmGetServiceThreadById( uint serviceId )
{
	if( !siList )
		GetServiceListPtr();
	if( serviceId< SI_MAX && siList[serviceId].ServiceId )
			return siList[serviceId].ThreadId;
	return 0;
}

uint SmGetServiceThreadByName( const char* name )
{
	int i;
	if( !siList )
		GetServiceListPtr();
	for( i=0; i<SI_MAX; i++ )
		if( strncmp(siList[i].ServiceName, name, SERVICE_NAME_LENGTH) == 0 ){
			return siList[i].ThreadId;
		}
	return 0;
}

