#include <sgos.h>
#include <stdio.h>
#include <string.h>
#include <system.h>

using namespace System;

/* speaker服务调用示例 
若要beep声，则发送一个如下的消息
<msg to="speaker">
	<beep />
</msg>
若要播放不同频率的声音，则如下
<msg to="speaker">
	<play freq=365 delay=300 />
	<play freq=2000 delay=500 />
	<play freq=365 delay=300 />
</msg>
若要播放一曲test音乐，则如下
<msg to="speaker">
	<test />
</msg>
*/

const char help_msg[] = "<msg><help>\
	<command id=\"beep\">Make a beep sound.</command>\
	<command id=\"play\">Play a sound with a given frequency and delay time.</command>\
	<command id=\"test\">Play a test music.</command>\
	</help></msg>";

/* 具体命令处理函数 */
extern void play(double freq, unsigned int delay);
extern void beep();
extern void test();
void parseCommand( Messenger& msgRecv )
{
	string cmd = msgRecv.readName(".");
	if( cmd == "beep" ){
		beep();
	}else if( cmd == "play" ){
		play( msgRecv.getUInt(":freq"), msgRecv.getUInt(":delay") );
	}else if( cmd == "test" ){
		test();
	}else{
		printf("[speaker]Unknown command: %s\n", cmd.cstr() );
	}
}

/* 服务主循环 
 * 某些服务可能需要多线程，便于并发处理消息。
 * 但speaker的资源只有1个，所以单个线程处理已经很适合。
*/
int startService()
{
	//接收消息使者
	Messenger msgRecv;
	//注册线程名称，其它程序便可以向此服务线程发送消息
	Thread current = Thread::ThisThread();
	current.createName("speaker");
	for(;;){
		//Pending for messenger
		msgRecv.receive();
		//处理每一个命令请求
		if( msgRecv.redir("...") ){
			do{
				parseCommand( msgRecv );
			}while( msgRecv.moveNext() ); //处理下一个请求
			//到这里，已经没有请求
		}
		
	}
	current.deleteName("speaker");
	return 0;
}

int main()
{
	printf("Starting speaker service ...\n");
	return startService();
}
