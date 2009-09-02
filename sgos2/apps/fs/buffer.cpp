// sgos2 Disk Buffer
// sgos2 磁盘缓冲区
// 090830 by Huang Guan

#include <sgos.h>
#include <stdio.h>
#include <stdlib.h>
#include <system.h>
#include <queue.h>
#include "vfs.h"
#include "buffer.h"

using namespace System;

queue_t	empty_buffer;

static void delete_buffer( const void* ptr )
{
	buffer_t* buf = (buffer_t*)ptr;
	free(buf->data);
	free(buf);
}

static int search_buffer( const void* ptr, const void* block )
{
	if( ((buffer_t*)ptr)->block == (size_t)block )
		return 1;
	// continue searching
	return 0;
}

// 向设备发送读取消息
static int readBlock( buffer_t *buf )
{
	Messenger msger;
	msger.parse("<msg></msg>");
	// 目标驱动
	msger.putString( ":to", buf->device->driver );
	// 读扇区操作
	msger.mkdir("ReadSector");
	// 设备ID
	msger.putUInt( ":part", buf->device->devID );
	// 读取扇区号
	msger.putUInt( ":start", BLOCK_TO_SECTOR(buf->block) );
	// 读取扇区数
	msger.putUInt( ":count", BLOCK_TO_SECTOR(1) );
	// 发送请求
	msger.request();
	if( msger.redir("/error") ){
		printf("[vfs]read Device error: %s\n", msger.getString("/error:string") );
		return -msger.getInt("/error:code");
	}else if(msger.redir("/ReadSector")){
		msger.read( ".", buf->data, BLOCK_SIZE );
	}
	return 0;
}

static int writeBlock( buffer_t * buf )
{
	Messenger msger;
	msger.parse("<msg></msg>");
	// 目标驱动
	msger.putString( ":to", buf->device->driver );
	// 扇区操作
	msger.mkdir("WriteSector");
	// 设备ID
	msger.putUInt( ":part", buf->device->devID );
	// 扇区号
	msger.putUInt( ":start", BLOCK_TO_SECTOR(buf->block) );
	// 扇区数
	msger.putUInt( ":count", BLOCK_TO_SECTOR(1) );
	// 数据内容
	msger.write( ".", buf->data, BLOCK_SIZE );
	// 发送请求
	msger.request();
	if( msger.redir("/error") ){
		printf("[vfs]write Device error: %s\n", msger.getString("/error:string") );
		return -msger.getInt("/error:code");
	}else {
		msger.redir("/WriteSector");
		if( strcmp(msger.getString(":result"), "ok")!=0 ){
			printf("[vfs]write Device failed.\n");
			return -ERR_UNKNOWN;
		}
	}
	return 0;
}

// 磁盘缓冲区初始化
int buffer_init( size_t buf_mem )
{
	int ret, blk_count;
	buffer_t *buf;
	ret = queue_create( &empty_buffer, 0, delete_buffer, "buffer_queue", 0 );
	if( ret<0 )
		return ret;
	blk_count = buf_mem>>BLOCK_SIZE_BITS;
	for( int i=0; i<blk_count; i++ ){
		buf = (buffer_t*)malloc(sizeof(buffer_t));
		if( buf == NULL ){
			return -ERR_NOMEM;
		}
		memset( buf, 0, sizeof(buffer_t) );
		buf->data = (uchar*)malloc( BLOCK_SIZE );
		if( buf == NULL ){
			return -ERR_NOMEM;
		}
		// put empty buffer to empty queue.
		queue_push_front( &empty_buffer, buf );
	}
	printf("[vfs]buffer_init OK!\n");
	return 0;
}

// 获取缓冲区
buffer_t* buffer_get( device_t* dev, size_t block )
{
	qnode_t* nod;
	buffer_t* buf;
	//临界区....
	buf = (buffer_t*)queue_search( &dev->bufferQueue, (void*)block, search_buffer, &nod );
	if( buf ){
		buf->reference ++;
		return buf;
	}
	//可以查找最近释放的几个块，看有没有需要的
	//若无，则随便获取一个空闲块
	buf = (buffer_t*)queue_pop_front( &empty_buffer );
	if( !buf ){
		printf("[vfs]buffer_get failed: no empty buffer.\n");
		return (buffer_t*)0;
	}
	if( buf->dirty )
		writeBlock(buf);
	buf->block = block;
	buf->device = dev;
	buf->reference = 0;
	buf->dirty = 0;
	//读取块数据
	if( readBlock( buf )<0 ){
		queue_push_front( &empty_buffer, buf );
		printf("[vfs]readBlock failed.\n");
		return (buffer_t*)0;
	}
	//添加到设备中
	queue_push_front( &dev->bufferQueue, buf );
	//设置引用计数
	buf->reference = 1;
	return buf;
}

// 释放缓冲区
void buffer_put( buffer_t* buf )
{
	qnode_t* nod;
	device_t* dev = buf->device;
	//临界区....
	buf = (buffer_t*)queue_quick_search( &dev->bufferQueue, (void*)buf, &nod );
	if( buf && buf->reference>0 ){
		buf->reference --;
		if( buf->reference == 0 ){
			queue_remove( &dev->bufferQueue, nod );
			queue_push_front( &empty_buffer, buf );
		}
	}
}

// 写入所有修改过的缓冲区
void buffer_sync()
{
	buffer_t* buf;
	int cont = 1;
	do{
		buf = (buffer_t*)queue_pop_front( &empty_buffer );
		if( buf->device ){
			if( buf->dirty )
				writeBlock(buf);
			buf->device = (device_t*)0;
		}else{
			cont = 0;
		}
		queue_push_back( &empty_buffer, buf );
	}while( cont );
}
