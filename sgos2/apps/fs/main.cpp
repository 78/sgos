
#include <sgos.h>
#include <stdio.h>
#include <system.h>
#include "buffer.h"
#include "vfs.h"


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
	//虚拟文件系统服务初始化
	ret = vfs_init();
	if( ret<0 ){
		printf("[vfs]vfs_init failed :%d\n", ret );
		return ret;
	}
	//
	for(;;){
		printf("-");
		System::Thread::Sleep(10000);
	}
	return 0;
}

