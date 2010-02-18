#include <sgos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <api.h>
#include "buffer.h"
#include "fsservice.h"

//
#define MAX_FS 16
fs_t *	fsList[MAX_FS];
int	fsCount = 0;

//安装一个文件系统类型
int fs_register( fs_t* fs )
{
	if( !fs )
		return -ERR_WRONGARG;
	if( fsCount >= MAX_FS ){
		printf("[vfs]too many fses.\n");
		return -ERR_NOMEM;
	}
	fsList[fsCount++] = fs;
	return 0;
}

//卸载文件系统类型
int fs_unregister( fs_t* fs )
{
	for(int i=0; i<fsCount; i++ )
		if( fsList[i] == fs ){
			if( i!=fsCount-1 )
				fsList[i] = fsList[fsCount-1];
			fsCount --;
		}
	return -ERR_WRONGARG;
}

//
static int try_fs( fs_t* fs, device_t* dev )
{
	dev->devFile = falloc();
	if( root ){
		++ root->reference;
		flinkto( dev->devFile, root );
	}
	// 尝试安装这个文件系统
	if( fs->setup && fs->setup( dev->devFile, dev )==0 ){
		//安装成功。
		dev->fs = fs;
		return 1;
	}
	ffree( dev->devFile );
	dev->devFile = 0;
	return 0;
}

//探测并安装文件系统
fs_t* fs_detect( device_t* dev )
{
	for(int i=0; i<fsCount; i++ )
		if( try_fs( fsList[i], dev ) )
			return dev->fs;
	return 0;
}

//文件系统服务初始化
int fs_init()
{
	fsCount = 0;
	memset( &fsList, 0 , sizeof(fsList) );
	return 0;
}
