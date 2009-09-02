#include <sgos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <system.h>
#include <queue.h>
#include "buffer.h"
#include "vfs.h"

//
queue_t fsQueue;

//安装一个文件系统类型
int fs_register( fs_t* fs )
{
	if( !fs )
		return -ERR_WRONGARG;
	queue_push_front( &fsQueue, fs );
	return 0;
}

//卸载文件系统类型
int fs_unregister( fs_t* fs )
{
	qnode_t* nod;
	fs = (fs_t*)queue_quick_search( &fsQueue, fs, &nod );
	if( fs ){
		queue_remove( &fsQueue, nod );
		return 0;
	}
	return -ERR_WRONGARG;
}

//
static int try_fs( const void* p, const void* q )
{
	fs_t* fs = (fs_t*)p;
	device_t* dev = (device_t*)q;
	file_t* file = &dev->devFile;
	// 尝试安装这个文件系统
	if( fs->setup && fs->setup( file, dev )==0 ){
		//安装成功。
		dev->fs = fs;
		return 1;
	}
	return 0;
}

//探测并安装文件系统
fs_t* fs_detect( device_t* dev )
{
	qnode_t *nod;
	//枚举所有的文件系统类型
	queue_search( &fsQueue, dev, try_fs, &nod );
	//
	return dev->fs;
}

//文件系统服务初始化
int fs_init()
{
	int ret;
	ret = queue_create( &fsQueue, 0, 0, "fsQueue", 0 );
	if( ret<0 )
		return ret;
	return 0;
}
