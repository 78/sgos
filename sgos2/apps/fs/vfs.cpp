/*
 *   Virtual File System
 *
 *   Author: Huang Guan  090830
 *
 */


#include <sgos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <system.h>
#include "buffer.h"
#include "vfs.h"

using namespace System;

//进程队列
queue_t procQueue;
//vfs服务线程
Thread* vfsThread;
//根目录文件
file_t* root;
device_t* rootDev;


static int search_process( const void* p, const void* q )
{
	if( ((procinfo_t*)p)->process == (uint)q )
		return 1;
	return 0;
}

procinfo_t* get_procinfo( uint proc )
{
	qnode_t *nod;
	procinfo_t* pinfo;
	pinfo = (procinfo_t*)queue_search( &procQueue, (void*)proc, search_process, &nod );
	return pinfo;
}

// 尝试打开一级目录
static int try_open( file_t* cur, file_t* next, const char* name )
{
	next->parent = cur;
	next->device = cur->device;
	if( !cur->dir && !(cur->attribute&FILE_ATTR_DIR))
		return -ERR_NOPATH;
	if( !cur->device->fs || !cur->device->fs->open )
		return -ERR_NOPATH;
	strncpy( next->name, name, FILE_NAME_LEN-1 );
	//调用文件系统操作
	return cur->device->fs->open( next, name );
}

// 循环打开路径目录
static int name_file( procinfo_t* pinfo, file_t* file, const char* path )
{
	char name[FILE_NAME_LEN];
	int i, j, ret;
	file_t cur, next;
	memcpy( &cur, root, sizeof(file_t) );
	memcpy( &next, file, sizeof(file_t) );
	for( i=0, j=0; path[i]!='\0'; i++ ){
		if( path[i]=='/' ){
			//获取完一级目录
			name[j]='\0';
			if( j==0 ){
				memcpy( &cur, root, sizeof(file_t) );
			}else{
				//打开一级目录
				ret = try_open( &cur, &next, name );
				if( ret<0 ){
					return ret;
				}
				memcpy( &cur, &next, sizeof(file_t) );
			}
			j=0;
		}else{
			name[j++] = path[i];
		}
	}
	name[j]='\0';
	ret = try_open( &cur, file, name );
	if( ret<0 )
		return ret;
	return 0;
}

int vfs_open( uint proc, const char* path, uint mode, uint flag )
{
	procinfo_t* pinfo;
	file_t* file;
	int ret;
	pinfo = get_procinfo(proc);
	//如果该进程第一次使用文件操作
	if( pinfo == NULL ){
		pinfo = (procinfo_t*)malloc(sizeof(procinfo_t));
		if( pinfo==NULL )
			return -ERR_NOMEM;
		memset(pinfo, 0, sizeof(procinfo_t));
		pinfo->process = proc;
		queue_push_front( &procQueue, pinfo );
	}
	for( int i=0; i<MAX_OPEN_FILE; i++ ){
		if( !pinfo->openList[i] ){
			file = (file_t*)malloc(sizeof(file_t));
			if( file == NULL )
				return -ERR_NOMEM;
			memset( file, 0, sizeof(file_t) );
			file->mode = mode;
			file->flag = flag;
			strncpy( file->path, path, PATH_LEN-1 );
			ret = name_file( pinfo, file, path );
			if(  ret < 0 ){
				free(file);
				return ret;
			}
			pinfo->openList[i] = file;
			// Success!
			return i;
		}
	}
	//没有可用的描述符
	return -ERR_NOMEM;
}


int vfs_setsize( uint proc, int id, size_t siz )
{
	file_t* file;
	procinfo_t* pinfo = get_procinfo(proc);
	//有效性检查
	if(pinfo==NULL || id<0 || id>=MAX_OPEN_FILE )
		return -ERR_WRONGARG;
	file = pinfo->openList[id];
	if( !file )
		return -ERR_WRONGARG;
	if( file->device->fs->ioctl )
		return file->device->fs->setsize( file, siz );
	return -ERR_NOIMP;
}

int vfs_rename( uint proc, int id, const char* name )
{
	file_t* file;
	procinfo_t* pinfo = get_procinfo(proc);
	//有效性检查
	if(pinfo==NULL || id<0 || id>=MAX_OPEN_FILE )
		return -ERR_WRONGARG;
	file = pinfo->openList[id];
	if( !file )
		return -ERR_WRONGARG;
	if( file->device->fs->ioctl )
		return file->device->fs->rename( file, (char*)name );
	return -ERR_NOIMP;
}

int vfs_readdir( uint proc, int id, void* buf, size_t siz )
{
	file_t* file;
	procinfo_t* pinfo = get_procinfo(proc);
	//有效性检查
	if(pinfo==NULL || id<0 || id>=MAX_OPEN_FILE )
		return -ERR_WRONGARG;
	file = pinfo->openList[id];
	if( !file )
		return -ERR_WRONGARG;
	if( file->device->fs->ioctl )
		return file->device->fs->dir( file, buf, siz );
	return -ERR_NOIMP;
}

int vfs_ioctl( uint proc, int id, uint cmd, uint arg )
{
	file_t* file;
	procinfo_t* pinfo = get_procinfo(proc);
	//有效性检查
	if(pinfo==NULL || id<0 || id>=MAX_OPEN_FILE )
		return -ERR_WRONGARG;
	file = pinfo->openList[id];
	if( !file )
		return -ERR_WRONGARG;
	if( file->device->fs->ioctl )
		file->device->fs->ioctl( file, cmd, arg );
	return -ERR_NOIMP;
}

int vfs_read( uint proc, int id, size_t count, uchar* buf )
{
	file_t* file;
	procinfo_t* pinfo = get_procinfo(proc);
	//有效性检查
	if(pinfo==NULL || id<0 || id>=MAX_OPEN_FILE )
		return -ERR_WRONGARG;
	file = pinfo->openList[id];
	if( !file )
		return -ERR_WRONGARG;
	if( file->device->fs->read )
		return file->device->fs->read( file, buf, count );
	return -ERR_NOIMP;
}

int vfs_seek( uint proc, int id, size_t step, size_t pos )
{
	file_t* file;
	procinfo_t* pinfo = get_procinfo(proc);
	//有效性检查
	if(pinfo==NULL || id<0 || id>=MAX_OPEN_FILE )
		return -ERR_WRONGARG;
	file = pinfo->openList[id];
	if( !file )
		return -ERR_WRONGARG;
	switch(pos)
	{
	case SEEK_CUR:
		file->pos += step;
		if( file->pos > file->size )
			file->pos = file->size;
		break;
	case SEEK_SET:
		file->pos = step;
		if( file->pos > file->size )
			file->pos = file->size;
		break;
	case SEEK_END:
		file->pos = file->size;
		file->pos += step;
		if( file->pos > file->size )
			file->pos = file->size;
		break;
	}
	return 0;
}

int vfs_write( uint proc, int id, size_t count, const uchar* buf )
{
	file_t* file;
	procinfo_t* pinfo = get_procinfo(proc);
	//有效性检查
	if(pinfo==NULL || id<0 || id>=MAX_OPEN_FILE )
		return -ERR_WRONGARG;
	file = pinfo->openList[id];
	if( !file )
		return -ERR_WRONGARG;
	if( file->device->fs->write )
		return file->device->fs->write( file, buf, count );
	return -ERR_NOIMP;
}

int vfs_close( uint proc, int id )
{
	file_t* file;
	procinfo_t* pinfo = get_procinfo(proc);
	//有效性检查
	if(pinfo==NULL || id<0 || id>=MAX_OPEN_FILE )
		return -ERR_WRONGARG;
	file = pinfo->openList[id];
	if( !file )
		return -ERR_WRONGARG;
	//调用文件系统操作
	if( file->device->fs )
		file->device->fs->close(file);
	//释放内存空间
	free( file );
	pinfo->openList[id] = (file_t*)0;
	return 0;
}

//VFS服务处理过程
static void vfs_service()
{
	Thread current = Thread::ThisThread();
	current.createName("vfs");
	Messenger msgRecv, msgSend;
	string cmd;
	int ret;
	uint proc;
	for(;;){
		if( msgRecv.receive()<0 ){
			printf("[vfs]vfs_service recv error.\n");;
			break;
		}
		msgSend = msgRecv.reply();
		msgSend.parse("<msg></msg>");
		msgRecv.redir("...");
		cmd = msgRecv.readName(".");
		proc = msgRecv.getSenderProcess();
		if( cmd=="open" ){
			ret = vfs_open( proc, msgRecv.getString(":file"), 
				msgRecv.getUInt(":mode"), msgRecv.getUInt(":flag") );
			if( ret<0 )
				msgSend.mkdir("/open?result=error");
			//设置返回值
			msgSend.putInt("/open:return", ret );
		}else if( cmd=="close" ){
			ret = vfs_close( proc, msgRecv.getInt(":id") );
			msgSend.putInt("/close:return", ret );
		}else if( cmd=="rename" ){
			ret = vfs_rename( proc, msgRecv.getInt(":id"), 
				msgRecv.getString(":name") );
			msgSend.putInt("/rename:return", ret );
		}else if( cmd=="ioctl" ){
			ret = vfs_ioctl( proc, msgRecv.getInt(":id"), 
				msgRecv.getUInt(":cmd"), msgRecv.getUInt(":arg") );
			msgSend.putInt("/ioctl:return", ret );
		}else if( cmd=="seek" ){
			ret = vfs_seek( proc, msgRecv.getInt(":id"), 
				msgRecv.getUInt(":step"), msgRecv.getUInt(":pos") );
			msgSend.putInt("/seek:return", ret );
		}else if( cmd=="setsize" ){
			ret = vfs_setsize( proc, msgRecv.getInt(":id"), 
				msgRecv.getUInt(":size") );
			msgSend.putInt("/setsize:return", ret );
		}else if( cmd=="read" ){
			size_t count = msgRecv.getUInt(":count");
			uchar* buff = (uchar*)malloc(count);
			if( buff ){
				ret = vfs_read( proc, msgRecv.getInt(":id"), 
					msgRecv.getUInt(":count"), buff );
			}else{
				ret = -ERR_NOMEM;
			}
			if( ret>0 ){
				msgRecv.write( "/read", buff, count );
			}
			free(buff);
			msgSend.putInt("/read:return", ret );
		}else if( cmd=="write" ){
			size_t count = msgRecv.getUInt(":count");
			uchar* buff = (uchar*)malloc(count);
			if( buff ){
				msgRecv.read( ".", buff, count );
				ret = vfs_write( proc, msgRecv.getInt(":id"), 
					msgRecv.getUInt(":count"), buff );
			}else{
				ret = -ERR_NOMEM;
			}
			free(buff);
			msgSend.putInt("/read:return", ret );
		}else if( cmd=="readall" ){
			int fd;
			fd = vfs_open( proc, msgRecv.getString(":file"), FILE_READ, 0 );
			if( !(fd<0) ){
				procinfo_t* pinfo = get_procinfo( proc );
				size_t flen = pinfo->openList[fd]->size;
				uchar* buff = (uchar*)malloc(flen+1);
				if( buff ){
					ret = vfs_read( proc, fd, flen, buff );
					if( ret >=0 ){
						buff[ret] = '\0';
						msgSend.write( "/readall", buff, ret+1 );
						msgSend.putUInt( "/readall:size", ret );
					}
					free(buff);
				}
				vfs_close( proc, fd );
			}else{
				fd = ret;
			}
			msgSend.putInt("/readall:return", ret );
		}else{
			printf("[vfs]Unknown vfs cmd: %s\n", cmd.cstr() );
		}
		msgSend.send();
	}
	current.deleteName("vfs");
	Thread::Exit(0);
}

// 虚拟文件系统服务初始化
extern fs_t fs_vfs;
extern fs_t fs_fat32;
int vfs_init()
{
	int ret;
	ret = queue_create( &procQueue, 0, 0, "procQueue", 0 );
	if( ret<0 )
		return ret;
	//创建服务线程
	vfsThread = new Thread( (void*)vfs_service );
	vfsThread->start();
	// 安装必要文件系统
	fs_register( &fs_vfs );
	// 注册设备以安装此文件系统
	rootDev = device_register( "root", 0 );
	if( rootDev==(device_t*)0 ){
		printf("[vfs]Failed to install root fs.\n");
	}else{
		root = &rootDev->devFile;
	}
	// 安装必要文件系统
	fs_register( &fs_fat32 );
	return 0;
}
