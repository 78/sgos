// sgos2 Disk Buffer
// sgos2 磁盘缓冲区
// 090830 by Huang Guan

#include <sgos.h>
#include <stdio.h>
#include <stdlib.h>
#include <api.h>
#include "fsservice.h"
#include "buffer.h"

buffer_t*	freebuf_head = 0;
buffer_t*	freebuf_tail = 0;

static void delete_buffer( const void* ptr )
{
	buffer_t* buf = (buffer_t*)ptr;
	SysFreeMemory(SysGetCurrentSpaceId(), buf->data);
	free(buf);
}

static buffer_t* freebuf_get()
{
	buffer_t* buf = freebuf_head;
	if( buf == 0 )
		return 0;
	if( buf->next )
		buf->next->prev = 0;
	else
		freebuf_tail = 0;
	freebuf_head = buf->next;
	return buf;
}

static void freebuf_put( buffer_t *buf )
{
	if( freebuf_tail )
		freebuf_tail->next = buf;
	buf->prev = freebuf_tail;
	buf->next = 0;
	freebuf_tail = buf;
	if( freebuf_head == 0 )
		freebuf_head = freebuf_tail;
}

// 向设备发送读取消息
static int readBlock( buffer_t *buf )
{
	Message msg;
	int result;
	memset( &msg, 0, sizeof(msg) );
	// 目标驱动
	msg.ThreadId = buf->device->thread;
	// 扇区操作
	msg.Command = Device_ReadSector;
	// 设备ID
	msg.Arguments[0] = buf->device->devID;
	// 扇区号
	msg.Arguments[1] = BLOCK_TO_SECTOR(buf->block);
	// 扇区数
	msg.Arguments[2] = BLOCK_TO_SECTOR(1);
	// 数据缓冲地址
	msg.Large[0] = (size_t)buf->data;
	// 发送请求
	result = Api_Send( &msg, 0 ); 
	if( result < 0 )
		return -ERR_UNKNOWN;
	result = Api_Receive( &msg, INFINITE );
	if( result < 0 ){
		printf("[vfs]receive message failed. \n");
		return result;
	}
	if( msg.Code < 0 ){
		printf("[vfs]read buffer failed. The device %d has a problem.\n", buf->device->devID);
		return msg.Code;
	}
	return 0;
}

static int writeBlock( buffer_t * buf )
{
	Message msg;
	int result;
	memset( &msg, 0, sizeof(msg) );
	// 目标驱动
	msg.ThreadId = buf->device->thread;
	// 扇区操作
	msg.Command = Device_WriteSector;
	// 设备ID
	msg.Arguments[0] = buf->device->devID;
	// 扇区号
	msg.Arguments[1] = BLOCK_TO_SECTOR(buf->block);
	// 扇区数
	msg.Arguments[2] = BLOCK_TO_SECTOR(1);
	// 数据内容
	msg.Large[0] = (size_t)buf->data;
	// 发送请求
	result = Api_Send( &msg, 0 ); 
	if( result < 0 )
		return -ERR_UNKNOWN;
	result = Api_Receive( &msg, INFINITE );
	if( result < 0 ){
		printf("[vfs]receive message failed. \n");
		return result;
	}
	if( msg.Code < 0 ){
		printf("[vfs]write buffer failed. The device %d has a problem.\n", buf->device->devID);
		return msg.Code;
	}
	return 0;
}

// 磁盘缓冲区初始化
int buffer_init( size_t buf_mem )
{
	int ret, blk_count;
	int sid = SysGetCurrentSpaceId();
	buffer_t *buf;
	freebuf_head = 
	freebuf_tail = 0;
	blk_count = buf_mem >> BLOCK_SIZE_BITS;
	for( int i=0; i<blk_count; i++ ){
		buf = (buffer_t*)malloc(sizeof(buffer_t));
		if( buf == 0 ){
			return -ERR_NOMEM;
		}
		memset( buf, 0, sizeof(buffer_t) );
		buf->data = (uchar*)SysAllocateMemory( sid, BLOCK_SIZE, MEMORY_ATTR_WRITE, ALLOC_SWAP );
		if( buf == 0 ){
			return -ERR_NOMEM;
		}
		// put empty buffer to empty chain. 
		freebuf_put( buf );
	}
	printf("[vfs]buffer_init OK!\n");
	return 0;
}

// 获取缓冲区
buffer_t* buffer_get( device_t* dev, size_t block )
{
	buffer_t* buf;
	//临界区....
	for( buf=dev->firstBuffer; buf; buf=buf->next ){
		if( buf->block == block ){
			buf->reference ++;
			return buf;
		}
	}
	//可以查找最近释放的几个块，看有没有需要的
	//Not implemented!
	//若无，则随便获取一个空闲块
	buf = freebuf_get();
	if( !buf ){
		printf("[vfs]buffer_get failed: no empty buffer.\n");
		return 0;
	}
	if( buf->dirty )
		writeBlock(buf);
	buf->block = block;
	buf->device = dev;
	buf->reference = 0;
	buf->dirty = 0;
	//读取块数据
	if( readBlock( buf )<0 ){
		freebuf_put( buf );
		printf("[vfs]readBlock failed.\n");
		return (buffer_t*)0;
	}
	//添加到设备中
	if( dev->firstBuffer )	
		dev->firstBuffer->prev = buf;
	buf->next = dev->firstBuffer;
	dev->firstBuffer = buf;
	//设置引用计数
	buf->reference = 1;
	return buf;
}

// 释放缓冲区
#define CHAIN_DELETE( i, l ) { \
	if( i->prev ){ i->prev->next = i->next; }else{ l = i->next; }	\
	if( i->next ){ i->next->prev = i->prev; } \
	}
void buffer_put( buffer_t* buf )
{
	device_t* dev = buf->device;
	if( buf->reference==0 )
		printf("[vfs]##BUG at buffer_put buf->reference==0\n");
	//临界区....
	buf->reference --;
	if( buf->reference == 0 ){
		CHAIN_DELETE( buf, dev->firstBuffer );
		freebuf_put( buf );
	}
}

// 写入所有修改过的缓冲区
void buffer_sync()
{
	buffer_t* buf;
	//临界区
	for( buf = freebuf_head; buf; buf=buf->next ){
		if( buf->device ){
			if( buf->dirty )
				writeBlock(buf);
			buf->device = (device_t*)0;
		}
	}
}
