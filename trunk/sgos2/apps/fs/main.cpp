
#include <sgos.h>
#include <stdio.h>
#include <api.h>
#include "buffer.h"
#include "fsservice.h"


int main()
{
	int ret;
	//磁盘缓冲区初始化
	ret = buffer_init( (1<<20) );
	if( ret<0 ){
		printf("[vfs]buffer_init failed :%d\n", ret );
		return ret;
	}
	//设备服务初始化
	ret = device_init();
	if( ret<0 ){
		printf("[vfs]device_init failed :%d\n", ret );
		return ret;
	}
	//动态文件系统装载服务初始化
	ret = fs_init();
	if( ret<0 ){
		printf("[vfs]fs_init failed :%d\n", ret );
		return ret;
	}
	//虚拟文件系统服务初始化, install rootfs!!
	ret = vfs_init();
	if( ret<0 ){
		printf("[vfs]rootfs_init failed :%d\n", ret );
		return ret;
	}
	extern void device_startService();
	device_startService();
	//
	SysExitThread(0);
	return 0;
}

