// sgos2 设备管理

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <system.h>
#include <queue.h>
#include "vfs.h"

using namespace System;

Thread* devThread;
queue_t devQueue;

/* 注册消息 
<msg to="dev">
	<register driver="hd" part=1 />
</msg>

*/

static int search_device( const void* p, const void* q )
{
	if( strcmp( ((device_t*)p)->name, (const char*)q)==0 )
		return 1;
	return 0;
}

// 由设备名获取设备
device_t* device_find( const char* devName )
{
	qnode_t* nod;
	device_t* dev;
	dev = (device_t*)queue_search( &devQueue, (void*)devName, search_device, &nod );
	return dev;
}

// 在文件系统中安装设备
int device_mount( device_t* dev, const char* path )
{
	uint proc = 0;
	int fd;
	fd = vfs_open( proc, path, FILE_READ, FILE_FLAG_CREATE );
	if( fd<0 ){
		printf("Failed to mount %s on %s\n", dev->name, path );
		return fd;
	}
	// 设置节点值为该设备的文件描述符
	vfs_ioctl( proc, fd, VFS_SET_VALUE, (uint)&dev->devFile );
	// 设置节点类型为文件系统
	vfs_ioctl( proc, fd, VFS_SET_TYPE, VFS_TYPE_FS );
	return 0;
}

// 检查是否已经注册过该设备
static int check_if_registered( const void* p, const void* q )
{
	if( strcmp( ((device_t*)p)->driver, ((device_t*)q)->driver )==0 &&
		((device_t*)p)->devID == ((device_t*)q)->devID )
		return 1;
	return 0;
}

// 注册设备
device_t* device_register( const char* driver, uint part )
{
	qnode_t* nod;
	device_t* dev = (device_t*)malloc( sizeof(device_t) );
	if( dev==NULL ){
		printf("[vfs]Failed to malloc memory for new device.\n");
		return (device_t*)0;
	}
	memset( dev, 0, sizeof(device_t) );
	strncpy( dev->driver, driver, NAME_LEN-1 );
	dev->devID = part;
	//检查该设备是否已经注册过。
	if( queue_search( &devQueue, dev, check_if_registered, &nod ) ){
		free(dev);
		return (device_t*)0;
	}
	// 初始化设备缓冲区
	queue_create( &dev->bufferQueue, 0, 0, "bufQueue", 0 );
	// 检测文件系统类型
	dev->fs = fs_detect( dev );
	if( strcmp( dev->driver, "root" )==0 ){
		strcpy( dev->name, "/" );
	}else{
		// 如果文件系统没有为设备分配设备名，则随机分配
		if( !dev->name[0] ){
			qnode_t* nod;
			dev->name[0]='c';
			dev->name[1]=':';
			for(;dev->name[0]<='z';dev->name[0]++ ){
				if( device_find( dev->name )==NULL )
					break;
			}
			// 如果得不到设备名怎么办？
			if( dev->name[0]>='z'){
				sprintf(dev->name, "%d", rand());
			}
			printf("[vfs]Registered dev %s for %s:%d\n", dev->name, driver, part );
		}
		if( !dev->fs ){
			printf("[vfs]Failed to install fs on %s:%d\n", dev->driver, dev->devID );
		}else{
			//把该设备安装到根目录下
			char path[NAME_LEN+10];
			sprintf(path, "/%s", dev->name );
			device_mount( dev, path );
		}
	}
	// 加入到队列中
	queue_push_front( &devQueue, dev );
	return dev;
}

// 设备注册服务
static void dev_service()
{
	printf("[vfs]dev_service started.\n");
	Messenger msgRecv;
	string cmd;
	int ret;
	Thread current = Thread::ThisThread();
	current.createName("dev");
	for(;;){
		ret = msgRecv.receive();
		if( ret<0 ){
			printf("[vfs]error in dev_service()\n");
			break;
		}
		msgRecv.redir("...");
		cmd = msgRecv.readName(".");
		if( cmd=="register" ){
			device_register( msgRecv.getString(":driver"),
				msgRecv.getUInt(":part") );
		}else if( cmd=="unregister" ){
			//Not implemented.
		}
	}
	current.deleteName("dev");
	Thread::Exit(0);
}

// 设备服务初始化
int device_init()
{
	int ret;
	ret = queue_create(&devQueue, 0, 0, "devQueue", 0 );
	if( ret<0 )
		return ret;
	devThread = new Thread( (void*)dev_service );
	devThread->start();
	printf("[vfs]device_init() OK!\n");
	return 0;
}

