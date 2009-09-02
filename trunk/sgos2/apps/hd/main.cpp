#include <stdio.h>
#include <system.h>

using namespace System;

/* hd服务调用示例 
读写磁盘消息
<msg to="hd">
	<ReadSector part=1 start=0 count=1 /> 
</msg>
<msg to="hd">
	<WriteSector part=1 start=0 count=1>
		...数据
	</WriteSector>
</msg>
*/

void replyError( Messenger& sender, int err_code, const char* err_str )
{
	sender.putString( "/error:string", err_str );
	sender.putInt( "/error:code", err_code );
}

/* 具体命令处理函数 */
extern int lba_init();
int lba_rw_sectors(t_8 dev, t_32 sec_start, t_32 sec_count, uchar *buf, int write);
void parseCommand( Messenger& msgRecv )
{
	t_8 dev;
	t_32 sec_start, sec_count;
	uchar *buf;
	msgRecv.redir("...");
	Messenger sender = msgRecv.reply();
	sender.parse("<msg/>");
	string cmd = msgRecv.readName(".");
	if( cmd == "ReadSector" ){
		dev = msgRecv.getUInt(":part");
		sec_start = msgRecv.getUInt(":start");
		sec_count = msgRecv.getUInt(":count");
		buf = new uchar[sec_count<<9];
		if( buf == NULL ){
			replyError( sender, ERR_NOMEM, "alloc failed." );
		}else{
			lba_rw_sectors( dev, sec_start, sec_count, buf, 0 );
			sender.write("/ReadSector?result=ok", buf, sec_count<<9 );
		}
		delete []buf;
	}else if( cmd == "WriteSector" ){
		dev = msgRecv.getUInt(":part");
		sec_start = msgRecv.getUInt(":start");
		sec_count = msgRecv.getUInt(":count");
		buf = new uchar[sec_count<<9];;
		if( buf == NULL ){
			replyError( sender, ERR_NOMEM, "alloc failed." );
		}else{
			msgRecv.read( ".", buf, sec_count<<9 );
			lba_rw_sectors( dev, sec_start, sec_count, buf, 1 );
			sender.mkdir("/WriteSector?result=ok");
		}
		delete []buf;
	}else if( cmd == "Initialize" ){
		lba_init();
		sender.mkdir("/Initialize?result=ok");
	}else{
		printf("[hd]Unknown command: %s\n", cmd.cstr() );
		replyError( sender, ERR_WRONGARG, "Unknown command." );
	}
	sender.send();
}

/* 服务主循环 
 * 某些服务可能需要多线程，便于并发处理消息。
 * 但speaker的资源只有1个，所以单个线程处理已经很适合。
*/
int startService()
{
	//接收消息使者
	Messenger msger;
	//注册线程名称，其它程序便可以向此服务线程发送消息
	Thread current = Thread::ThisThread();
	current.createName("hd");
	//初始化驱动程序
	lba_init();
	for(;;){
		//Pending for messenger
		msger.receive();
		parseCommand( msger );
		
	}
	current.deleteName("hd");
	return 0;
}

int main()
{
	printf("Starting hd service ...\n");
	return startService();
}

