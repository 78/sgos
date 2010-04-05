#include <stdio.h>
#include <string.h>
#include <sgos.h>
#include <api.h>

#define BUFFER_SIZE KB(64)
#define MAX_SECTORS  (BUFFER_SIZE>>9)
static uchar* bufferPage;

/* 具体命令处理函数 */
extern int lba_init();
int lba_rw_sectors(t_8 dev, t_32 sec_start, t_32 sec_count, uchar *buf, int write);
void parse( Message& msg )
{
	t_8 dev;
	t_32 sec_start, sec_count;
	int ret;
	switch( msg.Command ){
	case Device_ReadSector:
		dev = msg.Arguments[0];
		sec_start = msg.Arguments[1];
		sec_count = msg.Arguments[2];
	//	printf("[HD]read sector %d,%d,%d\n", dev, sec_start, sec_count );
		if( sec_count > MAX_SECTORS )
			sec_count = MAX_SECTORS;
		msg.Code = lba_rw_sectors( dev, sec_start, sec_count, bufferPage, 0 );
		ret = SysSwapMemory( SPACEID(msg.ThreadId), (size_t)msg.Large[0], (size_t)bufferPage,
			(sec_count<<9), //Bytes
			MAP_ADDRESS );
		break;
	case Device_WriteSector:
		dev = msg.Arguments[0];
		sec_start = msg.Arguments[1];
		sec_count = msg.Arguments[2];
	//	printf("[HD]write sector %d,%d,%d\n", dev, sec_start, sec_count );
		if( sec_count > MAX_SECTORS )
			sec_count = MAX_SECTORS;
		SysSwapMemory( SPACEID(msg.ThreadId), (size_t)msg.Large[0], (size_t)bufferPage,
			(sec_count<<9), //Bytes
			MAP_ADDRESS );
		msg.Code = lba_rw_sectors( dev, sec_start, sec_count, bufferPage, 1 );
		break;
	default:
		printf("[hd]Unknown command: %x\n", msg.Command );
		msg.Code = -ERR_WRONGARG;
	}
}

/* 服务主循环 
 * 某些服务可能需要多线程，便于并发处理消息。
*/
int startService()
{
	//接收消息
	Message msg;
	int id;
	//注册线程名称，其它程序便可以向此服务线程发送消息
	id = SmNotifyService( 0, 0, "HD" );
	if( id < 0 ){
		printf("[Harddisk]add service failed.\n");
		SysExitSpace((uint)-1);
	}
	//初始化驱动程序
	lba_init();
	//获取一个页面存放数据
	bufferPage = (uchar*)SysAllocateMemory( SysGetCurrentSpaceId(), BUFFER_SIZE, MEMORY_ATTR_WRITE, ALLOC_SWAP );
	if( bufferPage == NULL ){
		printf("[HD] bufferPage == NULL \n");
		SysExitSpace( (uint)-2);
	}
	printf("[HD]Starting hd service ...\n");
	for(;;){
		//Pending for messages
		int result = WaitMessage(&msg);
		if( result < 0 ){
			printf("[HD]Failed to receive message: result = %d\n", result );
			continue;
		}
		msg.Code = 0;
		parse( msg );
		ReplyMessage( &msg );
	}
	SmRemoveService( id );
	return 0;
}

int main()
{
	return startService();
}

