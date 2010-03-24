// sgos2 设备管理

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <api.h>
#include "fsservice.h"

#define MAX_DEVICE 256
uint deviceThread;
device_t * deviceList[MAX_DEVICE];
int	deviceCount = 0;

// 由设备名获取设备
device_t* device_find( const char* name )
{
	device_t* dev;
	for(int i=0; i<deviceCount; i++ ){
		dev = deviceList[i];
		if( dev->devFile->name!=name && strcmp( dev->devFile->name, name)==0)
			return dev;
	}
	return 0;
}

/*
// 在文件系统中安装设备
int device_mount( device_t* dev, const char* path )
{
	int fd;
	fd = vfs_open( path, FILE_READ, FILE_FLAG_CREATE );
	if( fd<0 ){
		printf("Failed to mount %s on %s\n", dev->devFile->name, path );
		return fd;
	}
	// 设置节点值为该设备的文件描述符
	vfs_ioctl( proc, fd, ROOTFS_SET_VALUE, (uint)dev->devFile );
	// 设置节点类型为文件系统
	vfs_ioctl( proc, fd, ROOTFS_SET_TYPE, ROOTFS_TYPE_FS );
	return 0;
}
*/

// 注册设备
device_t* device_register( uint type, uint thread, uint part, char* name )
{
	//检查该设备是否已经注册过。
	for( int i=0; i<deviceCount; i++ )
		if( deviceList[i]->thread == thread &&
			deviceList[i]->devID == part ){
			return 0; //Already registered
		}
	if( deviceCount >= MAX_DEVICE ){
		printf("[vfs] too many devices in the list.\n");
		return 0;
	}
	device_t* dev = (device_t*)malloc( sizeof(device_t) );
	deviceList[deviceCount] = dev;
	deviceCount ++;
	if( dev==0 ){
		printf("[vfs]Failed to malloc memory for new device %s.\n", name );
		return 0;
	}
	memset( dev, 0, sizeof(device_t) );
	strncpy( dev->driver, name, SERVICE_NAME_LENGTH-1 );
	dev->devID = part;
	dev->thread = thread;
	// 检测文件系统类型
	dev->fs = fs_detect( dev );
	if( dev->fs == 0 ){
		printf("[vfs]Failed to install fs on %s:%d\n", dev->driver, dev->devID );
	}else{
		file_t* f = dev->devFile;
		if( type == DEV_TYPE_ROOT ){
			strcpy( f->name, "/" );
		}else{
			// 如果文件系统没有为设备分配设备名，则随机分配
			if( !f->name[0] ){
				f->name[0]='c';
				f->name[1]=':';
				for(;f->name[0]<='z';f->name[0]++ ){
					if( device_find( f->name )==0 )
						break;
				}
				// 如果得不到设备名怎么办？
				if( f->name[0]>'z'){
					sprintf(f->name, "%s:%d", name, part);
				}
				printf("[vfs]Registered dev %s for %s:%d\n", f->name, name, part );
			}
			//把该设备安装到根目录下
		//	char path[NAME_LEN+10];
		//	sprintf(path, "/%s", f->name );
		//	device_mount( dev, path );
		}
	}
	return dev;
}


// 设备注册服务
static void dev_service()
{
	Message msg;
	int ret;
	ret = SmNotifyService( DeviceManagerId, 0, "DeviceManager" );
	if( ret < 0 ){
		printf("[vfs]notify service failed. ret=%d\n", ret );
		SysExitSpace((uint)-1);
	}
	printf("[vfs]dev_service started.\n");
	for(;;){
		memset( &msg, 0, sizeof(msg) );
		ret = WaitMessage(&msg);
		if( ret<0 ){
			printf("[vfs]receive error in dev_service()\n");
			continue;
		}
		msg.Code = 0;
		switch( msg.Command ){
		case Device_Register:
		{
			printf("[vfs]Register %s\n", (char*)&msg.Arguments[3] );
			ReplyMessage( &msg );
			device_t *d = device_register( 
				msg.Arguments[0], //Device Type
				msg.Arguments[1], //ServiceThreadId
				msg.Arguments[2], //Device No. 
				(char*)&msg.Arguments[3]  //Device Name  
				);
			break;
		}
		case Device_Unregister:
			//Not implemented.
			msg.Code = -ERR_NOIMP;
			ReplyMessage( &msg );
			break;
		}
	}
	SmRemoveService( DeviceManagerId );
	SysExitThread(0);
}

// 设备服务初始化
int device_init()
{
	deviceCount = 0;
	memset( deviceList, 0, sizeof(deviceList));
	deviceThread = SysCreateThread( SysGetCurrentSpaceId(), (size_t)&dev_service, 0, 0, 0 );
	return 0;
}

void device_startService()
{
	SysResumeThread(deviceThread);
}



