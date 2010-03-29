#include <sgos.h>
#include <api.h>
#include <stdio.h>
#include <string.h>

void beep();
void test();
void play(double freq, unsigned int delay);

void ServiceMessageLoop()
{
	Message msg;
	int result;
	printf("[Speaker]Service started.\n");
	for( ;; ){
		//Remember to reset msg every time.
		memset( &msg, 0, sizeof(msg) );
		result = WaitMessage( &msg );
		if( result < 0 ){
			printf("[Speaker] Failed in WaitMessage: result = %d\n", result );
			continue;
		}
		msg.Code = 0; //set it to success
		switch( msg.Command ){
		case 1:
			test();
			break;
		case 2:
			play( msg.Arguments[0], msg.Arguments[1] );
			break;
		case 3:
			beep();
			break;
		default:
			printf("[Speaker]Unknown command: %x\n", msg.Command );
		}
		ReplyMessage( &msg );
	}
}


int main()
{
	int id;
	//注册线程名称，其它程序便可以向此服务线程发送消息
	id = SmNotifyService( 0, 0, "Speaker" );
	if( id < 0 ){
		printf("[Speaker]add service failed.\n");
		return(-1);
	}
	//Do Service Loop
	ServiceMessageLoop();
	return 0;
}

